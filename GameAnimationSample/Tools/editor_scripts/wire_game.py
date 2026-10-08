import unreal, json, traceback
log = {}
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
eal = unreal.EditorAssetLibrary

def step(name):
    def deco(fn):
        try:
            r = fn()
            log[name] = "ok" if r is None else r
        except Exception as e:
            log[name] = "ERR " + str(e)[:300] + " | " + traceback.format_exc()[-300:]
        return fn
    return deco

def mkdir(p):
    if not eal.does_directory_exist(p):
        eal.make_directory(p)

for d in ("/Game/RR", "/Game/RR/Player", "/Game/RR/Blueprints", "/Game/RR/Levels"):
    mkdir(d)

PA = "/Game/RR/Player/Anims/"
PM = "/Game/RR/Player/"

def montage_from(seq_path, name, folder=PM):
    path = folder + name
    if eal.does_asset_exist(path):
        return unreal.load_asset(path)
    s = unreal.load_asset(seq_path)
    f = unreal.AnimMontageFactory()
    f.set_editor_property("source_animation", s)
    m = at.create_asset(name, folder.rstrip("/"), unreal.AnimMontage, f)
    eal.save_asset(path)
    return m

def bp_template(bp, class_substr):
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    for h in sub.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if obj and class_substr in obj.get_class().get_name():
            return obj
    return None

def set_hit_reactions(comp, mapping):
    hr = comp.get_editor_property("hit_reactions")
    for k, v in mapping.items():
        hr.set_editor_property(k, v)
    comp.set_editor_property("hit_reactions", hr)

def compile_save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)

# ---------------------------------------------------------------- player montages + weapon
M = {}
@step("player_montages")
def _():
    names = {"AM_P_Light1": "RT_Attack_PrimaryA", "AM_P_Light2": "RT_Attack_PrimaryB", "AM_P_Light3": "RT_Attack_PrimaryC",
             "AM_P_Heavy": "RT_Attack_RMB", "AM_P_Block": "RT_Deflect_InPlace",
             "AM_P_HitReact_Front": "RT_HitReact_Front", "AM_P_HitReact_Back": "RT_HitReact_Back",
             "AM_P_HitReact_Left": "RT_HitReact_Left", "AM_P_HitReact_Right": "RT_HitReact_Right",
             "AM_P_Death": "RT_Death", "AM_P_Stun": "RT_Stun_Start"}
    for mn, sn in names.items():
        M[mn] = montage_from(PA + sn, mn)
    return {k: (v.get_name() if v else None) for k, v in M.items()}

@step("player_weapon_da")
def _():
    wpath = PM + "DA_PlayerWeapon"
    wcls = unreal.load_class(None, "/Script/AAACombat.AAAWeaponSettings")
    if eal.does_asset_exist(wpath):
        da = unreal.load_asset(wpath)
    else:
        fac = unreal.DataAssetFactory(); fac.set_editor_property("data_asset_class", wcls)
        da = at.create_asset("DA_PlayerWeapon", PM.rstrip("/"), wcls, fac)
    anims = da.get_editor_property("animations")
    anims.set_editor_property("light_attack_combos", [M["AM_P_Light1"], M["AM_P_Light2"], M["AM_P_Light3"]])
    anims.set_editor_property("heavy_attack_combos", [M["AM_P_Heavy"]])
    for prop in ("block_montage", "parry_montage"):
        try: anims.set_editor_property(prop, M["AM_P_Block"])
        except Exception as e: log["warn_" + prop] = str(e)[:100]
    da.set_editor_property("animations", anims)
    combat = da.get_editor_property("combat")
    combat.set_editor_property("base_damage", 16.0)
    combat.set_editor_property("range", 220.0)
    da.set_editor_property("combat", combat)
    abil = da.get_editor_property("abilities")
    mc = unreal.load_class(None, "/Script/AAAAbility.AAAGameplayAbility_Melee")
    abil.set_editor_property("granted_abilities", [mc])
    da.set_editor_property("abilities", abil)
    eal.save_asset(wpath)

@step("player_bp_hit_reactions")
def _():
    bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    comp = bp_template(bp, "CombatComponent")
    set_hit_reactions(comp, {"front": M["AM_P_HitReact_Front"], "back": M["AM_P_HitReact_Back"],
                             "left": M["AM_P_HitReact_Left"], "right": M["AM_P_HitReact_Right"],
                             "stagger": M["AM_P_Stun"], "death": M["AM_P_Death"]})
    compile_save(bp)

# ---------------------------------------------------------------- enemy blueprint
@step("enemy_bp")
def _():
    path = "/Game/RR/Blueprints/BP_RREnemy_Greystone"
    if eal.does_asset_exist(path):
        bp = unreal.load_asset(path)
    else:
        f = unreal.BlueprintFactory(); f.set_editor_property("parent_class", unreal.RREnemy)
        bp = at.create_asset("BP_RREnemy_Greystone", "/Game/RR/Blueprints", unreal.Blueprint, f)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    mesh = cdo.get_editor_property("mesh")
    gs = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Meshes/Greystone")
    abp = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint")
    mesh.set_editor_property("skeletal_mesh_asset", gs)
    mesh.set_editor_property("animation_mode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
    mesh.set_editor_property("anim_class", abp.generated_class())
    mesh.set_editor_property("relative_location", unreal.Vector(0, 0, -96))
    mesh.set_editor_property("relative_rotation", unreal.Rotator(0, 0, -90))
    cap = cdo.get_editor_property("capsule_component")
    cap.set_editor_property("capsule_half_height", 96.0)
    cap.set_editor_property("capsule_radius", 42.0)
    cdo.set_editor_property("weapon_settings", unreal.load_asset("/Game/RR/Greystone/DA_GreystoneWeapon"))
    comp = cdo.get_editor_property("combat_component")
    G = "/Game/RR/Greystone/"
    set_hit_reactions(comp, {k: unreal.load_asset(G + n) for k, n in
                             {"front": "AM_GS_HitReact_Front", "back": "AM_GS_HitReact_Back", "left": "AM_GS_HitReact_Left",
                              "right": "AM_GS_HitReact_Right", "stagger": "AM_GS_Stun", "death": "AM_GS_Death"}.items()})
    compile_save(bp)
    return "ai_controller_class=" + str(cdo.get_editor_property("ai_controller_class"))

# ---------------------------------------------------------------- game mode blueprint
@step("gamemode_bp")
def _():
    path = "/Game/RR/Blueprints/BP_RRGameMode"
    if eal.does_asset_exist(path):
        bp = unreal.load_asset(path)
    else:
        f = unreal.BlueprintFactory(); f.set_editor_property("parent_class", unreal.RRGameMode)
        bp = at.create_asset("BP_RRGameMode", "/Game/RR/Blueprints", unreal.Blueprint, f)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    pawn = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    enemy = unreal.load_asset("/Game/RR/Blueprints/BP_RREnemy_Greystone")
    cdo.set_editor_property("default_pawn_class", pawn.generated_class())
    cdo.set_editor_property("enemy_class", enemy.generated_class())
    cdo.set_editor_property("player_weapon_settings", unreal.load_asset(PM + "DA_PlayerWeapon"))
    compile_save(bp)
    return "hud=%s pc=%s ps=%s" % (cdo.get_editor_property("hud_class"), cdo.get_editor_property("player_controller_class"),
                                  cdo.get_editor_property("player_state_class"))

# ---------------------------------------------------------------- arena level
@step("arena_level")
def _():
    src, dst = "/Game/AAA/Levels/L_Developer", "/Game/RR/Levels/L_RRArena"
    if not eal.does_asset_exist(dst):
        eal.duplicate_asset(src, dst)
    unreal.EditorLoadingAndSavingUtils.load_map(dst)
    gm = unreal.load_asset("/Game/RR/Blueprints/BP_RRGameMode")
    ws = unreal.EditorLevelLibrary.get_editor_world().get_world_settings()
    ws.set_editor_property("default_game_mode", gm.generated_class())
    info = []
    for a in unreal.EditorLevelLibrary.get_all_level_actors():
        if a.get_actor_label() == "Plane":
            s = a.get_actor_scale3d()
            info.append("plane scale %.1f,%.1f,%.1f" % (s.x, s.y, s.z))
            if s.x < 40:
                a.set_actor_scale3d(unreal.Vector(60, 60, 1))
    has_nav = any(a.get_class().get_name() == "NavMeshBoundsVolume" for a in unreal.EditorLevelLibrary.get_all_level_actors())
    if not has_nav:
        v = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(840, 0, 0))
        v.set_actor_scale3d(unreal.Vector(40, 40, 10))
        info.append("nav volume added")
    unreal.EditorLevelLibrary.save_current_level()
    return info

open(unreal.Paths.project_saved_dir() + "RRTools/wire_out.json", "w").write(json.dumps(log, indent=1, default=str))
unreal.log("RR_WIRE_DONE")
