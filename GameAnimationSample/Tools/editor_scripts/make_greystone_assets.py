import unreal, json
log = {}
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
GS = "/Game/ParagonGreystone/Characters/Heroes/Greystone"
OUT = "/Game/RR/Greystone"
if not unreal.EditorAssetLibrary.does_directory_exist(OUT):
    unreal.EditorAssetLibrary.make_directory(OUT)

def seq(name):
    flt = unreal.ARFilter(package_paths=[GS + "/Animations"], recursive_paths=True)
    for a in ar.get_assets(flt):
        if str(a.asset_name) == name and str(a.asset_class_path.asset_name) == "AnimSequence":
            return a.get_asset()
    return None

def montage_from(seq_name, mont_name):
    path = OUT + "/" + mont_name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    s = seq(seq_name)
    if not s:
        log[mont_name] = "missing sequence " + seq_name
        return None
    f = unreal.AnimMontageFactory()
    f.set_editor_property("source_animation", s)
    m = at.create_asset(mont_name, OUT, unreal.AnimMontage, f)
    if m:
        unreal.EditorAssetLibrary.save_asset(path)
        log[mont_name] = "created from " + seq_name
    else:
        log[mont_name] = "FAILED create"
    return m

for sn, mn in [("HitReact_Front","AM_GS_HitReact_Front"),("HitReact_Back","AM_GS_HitReact_Back"),
               ("HitReact_Left","AM_GS_HitReact_Left"),("HitReact_Right","AM_GS_HitReact_Right"),
               ("Death","AM_GS_Death"),("Stun_Start","AM_GS_Stun"),("Attack_RMB","AM_GS_Heavy"),
               ("Deflect_InPlace","AM_GS_Deflect")]:
    montage_from(sn, mn)

def find_montage(name):
    flt = unreal.ARFilter(package_paths=[GS], recursive_paths=True)
    for a in ar.get_assets(flt):
        if str(a.asset_name) == name and str(a.asset_class_path.asset_name) == "AnimMontage":
            return a.get_asset()
    return None

combo = [find_montage(n) for n in ["Attack_PrimaryA_Montage","Attack_PrimaryB_Montage","Attack_PrimaryC_Montage"]]
log["combo_found"] = [str(c.get_name()) if c else None for c in combo]

# Weapon data asset for the Greystone enemy
wpath = OUT + "/DA_GreystoneWeapon"
wcls = unreal.load_class(None, "/Script/AAACombat.AAAWeaponSettings")
if wcls and not unreal.EditorAssetLibrary.does_asset_exist(wpath):
    fac = unreal.DataAssetFactory()
    fac.set_editor_property("data_asset_class", wcls)
    da = at.create_asset("DA_GreystoneWeapon", OUT, wcls, fac)
else:
    da = unreal.load_asset(wpath)
if da:
    try:
        anims = da.get_editor_property("animations")
        anims.set_editor_property("light_attack_combos", [c for c in combo if c])
        heavy = unreal.load_asset(OUT + "/AM_GS_Heavy")
        anims.set_editor_property("heavy_attack_combos", [heavy] if heavy else [])
        da.set_editor_property("animations", anims)
        combat = da.get_editor_property("combat")
        combat.set_editor_property("base_damage", 14.0)
        combat.set_editor_property("range", 190.0)
        da.set_editor_property("combat", combat)
        abil = da.get_editor_property("abilities")
        mc = unreal.load_class(None, "/Script/AAAAbility.AAAGameplayAbility_Melee")
        abil.set_editor_property("granted_abilities", [mc] if mc else [])
        da.set_editor_property("abilities", abil)
        unreal.EditorAssetLibrary.save_asset(wpath)
        log["DA_GreystoneWeapon"] = "ok melee_class=" + str(mc)
    except Exception as e:
        log["DA_GreystoneWeapon"] = "ERR " + str(e)[:200]
else:
    log["DA_GreystoneWeapon"] = "not created (class missing?)"

# Player-side facts for the retarget step
try:
    bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    cdo = unreal.get_default_object(bp.generated_class())
    sm = cdo.get_editor_property("mesh") if hasattr(cdo, "mesh") else None
    log["player_cdo_has_mesh"] = str(sm)
except Exception as e:
    log["player_mesh"] = "ERR " + str(e)[:100]
flt = unreal.ARFilter(package_paths=["/Game/Characters","/Game/AAA/Characters"], recursive_paths=True)
log["rig_assets"] = [str(a.package_name) + " [" + str(a.asset_class_path.asset_name) + "]" for a in ar.get_assets(flt) if str(a.asset_class_path.asset_name) in ("IKRigDefinition","IKRetargeter","Skeleton","SkeletalMesh")][:40]
open(unreal.Paths.project_saved_dir() + "RRTools/make_greystone_out.json", "w").write(json.dumps(log, indent=1, default=str))
unreal.log("RR_MAKE_DONE")
