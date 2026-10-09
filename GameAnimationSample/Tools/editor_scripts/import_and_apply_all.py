import unreal, os

print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()

WS = "A:/UNREAL PROJECT/Game/GameAnimationSample"

# =========================================================================
# 1. IMPORT COLOSSEUM FBX
# =========================================================================
col_fbx = f"{WS}/ExternalModels/Colosseum/source/C.fbx"
col_dst = "/Game/RR/Environment/Colosseum"
col_mesh_path = f"{col_dst}/SM_Colosseum"

if not eal.does_asset_exist(col_mesh_path):
    print("Importing Colosseum FBX...")
    t = unreal.AssetImportTask()
    t.filename = col_fbx
    t.destination_path = col_dst
    t.destination_name = "SM_Colosseum"
    t.automated = True
    t.save = True
    t.replace_existing = True
    
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = False
    ui.import_materials = True
    ui.import_textures = True
    ui.static_mesh_import_data.combine_meshes = True
    ui.static_mesh_import_data.generate_lightmap_u_vs = True
    t.options = ui
    
    at.import_asset_tasks([t])
    print("Colosseum imported:", t.imported_object_paths)
else:
    print("SM_Colosseum already exists!")

# =========================================================================
# 2. IMPORT FAB SWORD & SHIELD (GLB)
# =========================================================================
fab_dst = "/Game/RR/Weapons/Fab"
for name, glb_file in [("SM_LongSword", "long_sword.glb"), ("SM_VikingShield", "viking_shield.glb")]:
    target_path = f"{fab_dst}/{name}"
    if not eal.does_asset_exist(target_path):
        glb_path = f"{WS}/ExternalModels/Fab/{glb_file}"
        print(f"Importing {name} from {glb_file}...")
        t = unreal.AssetImportTask()
        t.filename = glb_path
        t.destination_path = fab_dst
        t.destination_name = name
        t.automated = True
        t.save = True
        t.replace_existing = True
        at.import_asset_tasks([t])
        print(f"{name} imported:", t.imported_object_paths)
    else:
        print(f"{target_path} already exists!")

# =========================================================================
# 3. REPLACE ARENA CYLINDER & PLATFORM IN L_RRArena WITH SM_Colosseum
# =========================================================================
LEVEL = "/Game/RR/Levels/L_RRArena"
print(f"Loading map {LEVEL}...")
unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
acts = eas.get_all_level_actors()

# Find existing Cylinder
cyls = [a for a in acts if a.get_actor_label() == "Cylinder" or "Cylinder" in a.get_actor_label()]
CX, CY, CZ = 3750.0, -910.0, 180.0
if cyls:
    loc = cyls[0].get_actor_location()
    CX, CY, CZ = loc.x, loc.y, loc.z
    print(f"Found old Cylinder at ({CX}, {CY}, {CZ}). Removing old cylinder...")
    eas.destroy_actor(cyls[0])

# Spawn or replace Colosseum StaticMeshActor
col_mesh = unreal.load_asset(col_mesh_path)
if col_mesh:
    existing_col = [a for a in acts if a.get_actor_label() == "RR_Colosseum"]
    for a in existing_col:
        eas.destroy_actor(a)
        
    col_actor = eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(CX, CY, CZ), unreal.Rotator(0, 0, 0))
    col_actor.set_actor_label("RR_Colosseum")
    smc = col_actor.static_mesh_component
    smc.set_static_mesh(col_mesh)
    smc.set_mobility(unreal.ComponentMobility.STATIC)
    print(f"Spawned RR_Colosseum at ({CX}, {CY}, {CZ})")

unreal.EditorLoadingAndSavingUtils.save_current_level()
print("Saved level L_RRArena with new Colosseum!")

# =========================================================================
# 4. IMPORT MIXAMO PRO SWORD & SHIELD ANIMATIONS
# =========================================================================
anim_src_dir = f"{WS}/ExternalAnims/ProSwordShield"
anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"

base_mesh_path = f"{anim_dst_dir}/SKM_ProSwordShield_Base"
skel_mesh = None

for a in ar.get_assets(unreal.ARFilter(package_paths=[anim_dst_dir], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "SkeletalMesh")])):
    skel_mesh = a.get_asset()
    break

if not skel_mesh:
    ch_fbx = f"{anim_src_dir}/Ch01_nonPBR.fbx"
    if os.path.exists(ch_fbx):
        print("Importing base character mesh...")
        t = unreal.AssetImportTask()
        t.filename = ch_fbx
        t.destination_path = anim_dst_dir
        t.destination_name = "SKM_ProSwordShield_Base"
        t.automated = True
        t.save = True
        t.replace_existing = True
        
        ui = unreal.FbxImportUI()
        ui.import_mesh = True
        ui.import_animations = False
        ui.import_materials = False
        ui.import_textures = False
        ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
        t.options = ui
        at.import_asset_tasks([t])
        
        for p in t.imported_object_paths:
            o = unreal.load_asset(p)
            if isinstance(o, unreal.SkeletalMesh):
                skel_mesh = o
                break

if skel_mesh:
    skeleton = skel_mesh.get_editor_property("skeleton")
    print(f"ProSwordShield SkeletalMesh: {skel_mesh.get_path_name()}, Skeleton: {skeleton.get_path_name()}")
    
    anim_files = [f for f in os.listdir(anim_src_dir) if f.endswith(".fbx") and not f.startswith("Ch01")]
    import_tasks = []
    
    for af in anim_files:
        asset_name = af.replace(".fbx", "").replace(" ", "_").replace("(", "").replace(")", "")
        dest_pkg = f"{anim_dst_dir}/{asset_name}"
        if not eal.does_asset_exist(dest_pkg):
            t = unreal.AssetImportTask()
            t.filename = f"{anim_src_dir}/{af}"
            t.destination_path = anim_dst_dir
            t.destination_name = asset_name
            t.automated = True
            t.save = True
            t.replace_existing = True
            
            ui = unreal.FbxImportUI()
            ui.import_mesh = False
            ui.import_animations = True
            ui.skeleton = skeleton
            t.options = ui
            import_tasks.append(t)
            
    if import_tasks:
        print(f"Importing {len(import_tasks)} animations...")
        at.import_asset_tasks(import_tasks)
        print("Animation import batch complete!")
        
    rig_path = f"{anim_dst_dir}/IK_ProSwordShield"
    rig = unreal.load_asset(rig_path) if eal.does_asset_exist(rig_path) else at.create_asset("IK_ProSwordShield", anim_dst_dir, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    rc = unreal.IKRigController.get_controller(rig)
    rc.set_skeletal_mesh(skel_mesh)
    rc.apply_auto_generated_retarget_definition()
    eal.save_asset(rig_path)
    
    ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
    ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
    rtg_path = f"{anim_dst_dir}/RTG_Pro_to_UEFN"
    rtg = unreal.load_asset(rtg_path) if eal.does_asset_exist(rtg_path) else at.create_asset("RTG_Pro_to_UEFN", anim_dst_dir, unreal.IKRetargeter, unreal.IKRetargetFactory())
    
    c = unreal.IKRetargeterController.get_controller(rtg)
    S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
    c.set_ik_rig(S, rig)
    c.set_ik_rig(T, ue_rig)
    c.set_preview_mesh(S, skel_mesh)
    c.set_preview_mesh(T, ue_mesh)
    if c.get_num_retarget_ops() == 0:
        c.add_default_ops()
    c.assign_ik_rig_to_all_ops(S, rig)
    c.assign_ik_rig_to_all_ops(T, ue_rig)
    c.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
    eal.save_asset(rtg_path)
    print("Retargeter RTG_Pro_to_UEFN configured and saved!")
    
    player_anims_dst = "/Game/RR/Player/Anims"
    key_combos = [
        "sword_and_shield_slash",
        "sword_and_shield_slash_2",
        "sword_and_shield_slash_3",
        "sword_and_shield_slash_4",
        "sword_and_shield_slash_5",
        "sword_and_shield_attack",
        "sword_and_shield_attack_2",
        "sword_and_shield_attack_3",
        "sword_and_shield_attack_4",
        "sword_and_shield_kick",
        "sword_and_shield_power_up",
        "sword_and_shield_block",
        "sword_and_shield_block_idle",
        "sword_and_shield_impact",
        "sword_and_shield_impact_2"
    ]
    
    anims_to_retarget = []
    for kc in key_combos:
        src_anim = unreal.load_asset(f"{anim_dst_dir}/{kc}")
        out_anim_path = f"{player_anims_dst}/RT_Pro_{kc}"
        if src_anim and not eal.does_asset_exist(out_anim_path):
            anims_to_retarget.append(src_anim)
            
    if anims_to_retarget:
        print(f"Retargeting {len(anims_to_retarget)} combo animations to player...")
        retargeted = unreal.IKRetargetBatchOperation().duplicate_and_retarget(
            anims_to_retarget, skel_mesh, ue_mesh, rtg, "", "", "RT_Pro_", "", player_anims_dst, False, False, True
        )
        for a in retargeted:
            eal.save_asset(str(a.package_name))
        print(f"Retargeted {len(retargeted)} player combat animations!")
        
    def mk_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0):
        path = f"/Game/RR/Player/{montage_name}"
        seq = unreal.load_asset(f"/Game/RR/Player/Anims/{seq_name}")
        if not seq:
            return None
        if eal.does_asset_exist(path):
            m = unreal.load_asset(path)
        else:
            f = unreal.AnimMontageFactory()
            f.set_editor_property("source_animation", seq)
            m = at.create_asset(montage_name, "/Game/RR/Player", unreal.AnimMontage, f)
        if m:
            b = m.get_editor_property("blend_in")
            b.set_editor_property("blend_time", bi)
            m.set_editor_property("blend_in", b)
            b = m.get_editor_property("blend_out")
            b.set_editor_property("blend_time", bo)
            m.set_editor_property("blend_out", b)
            m.set_editor_property("rate_scale", rate)
            eal.save_loaded_asset(m)
        return m

    # Light combo montages (5-step progressive chain)
    l1 = mk_montage("RT_Pro_sword_and_shield_slash", "AM_P_Pro_L1", 0.05, 0.15, 1.1)
    l2 = mk_montage("RT_Pro_sword_and_shield_slash_2", "AM_P_Pro_L2", 0.05, 0.15, 1.15)
    l3 = mk_montage("RT_Pro_sword_and_shield_slash_3", "AM_P_Pro_L3", 0.05, 0.15, 1.2)
    l4 = mk_montage("RT_Pro_sword_and_shield_slash_4", "AM_P_Pro_L4", 0.05, 0.15, 1.25)
    l5 = mk_montage("RT_Pro_sword_and_shield_slash_5", "AM_P_Pro_L5", 0.05, 0.25, 1.3)

    # Heavy combo montages
    h1 = mk_montage("RT_Pro_sword_and_shield_attack", "AM_P_Pro_H1", 0.08, 0.2, 1.0)
    h2 = mk_montage("RT_Pro_sword_and_shield_attack_2", "AM_P_Pro_H2", 0.08, 0.2, 1.05)
    h3 = mk_montage("RT_Pro_sword_and_shield_kick", "AM_P_Pro_Kick", 0.08, 0.2, 1.1)
    h4 = mk_montage("RT_Pro_sword_and_shield_power_up", "AM_P_Pro_PowerUp", 0.1, 0.25, 1.0)

    # Block & impact montages
    blk = mk_montage("RT_Pro_sword_and_shield_block", "AM_P_Pro_Block", 0.05, 0.1, 1.0)
    imp = mk_montage("RT_Pro_sword_and_shield_impact", "AM_P_Pro_Impact", 0.05, 0.1, 1.0)

    # Update DA_PlayerWeapon
    da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
    if da:
        anims = da.get_editor_property("animations")
        light_chain = [x for x in [l1, l2, l3, l4, l5] if x]
        heavy_chain = [x for x in [h1, h2, h3, h4] if x]
        if light_chain:
            anims.set_editor_property("light_attack_combos", light_chain)
        if heavy_chain:
            anims.set_editor_property("heavy_attack_combos", heavy_chain)
        if blk:
            anims.set_editor_property("block_montage", blk)
        da.set_editor_property("animations", anims)
        eal.save_loaded_asset(da)
        print("Updated DA_PlayerWeapon with new 5-hit light combo & heavy combo chains!")

print("IMPORT_AND_APPLY_ALL_COMPLETE")
