import unreal, os, json

unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
print("=== IMPORTING AND RETARGETING PRO SWORD AND SHIELD ANIMATIONS ===")

WS = "A:/UNREAL PROJECT/Game/GameAnimationSample"
anim_src_dir = f"{WS}/ExternalAnims/ProSwordShield"
anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"
player_montages_dst = "/Game/RR/Player"

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()

skel_path = f"{anim_dst_dir}/SKM_ProSwordShield_Base_Skeleton"
skeleton = unreal.load_asset(skel_path)
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")

key_anims = [
    ("sword and shield slash.fbx", "Pro_Slash_1"),
    ("sword and shield slash (2).fbx", "Pro_Slash_2"),
    ("sword and shield slash (3).fbx", "Pro_Slash_3"),
    ("sword and shield slash (4).fbx", "Pro_Slash_4"),
    ("sword and shield slash (5).fbx", "Pro_Slash_5"),
    ("sword and shield attack.fbx", "Pro_Attack_1"),
    ("sword and shield attack (2).fbx", "Pro_Attack_2"),
    ("sword and shield kick.fbx", "Pro_Kick"),
    ("sword and shield power up.fbx", "Pro_PowerUp"),
    ("sword and shield block.fbx", "Pro_Block"),
    ("sword and shield impact.fbx", "Pro_Impact"),
    ("draw sword 1.fbx", "Pro_Draw"),
    ("sheath sword 1.fbx", "Pro_Sheath"),
]

# 1. Import
import_tasks = []
for fbx_name, asset_name in key_anims:
    dest_pkg = f"{anim_dst_dir}/{asset_name}"
    fbx_file = f"{anim_src_dir}/{fbx_name}"
    if not os.path.exists(fbx_file):
        print(f"Missing file: {fbx_file}")
        continue
    if not eal.does_asset_exist(dest_pkg):
        t = unreal.AssetImportTask()
        t.filename = fbx_file
        t.destination_path = anim_dst_dir
        t.destination_name = asset_name
        t.automated = True
        t.save = True
        t.replace_existing = True
        
        ui = unreal.FbxImportUI()
        ui.import_mesh = False
        ui.import_animations = True
        ui.skeleton = skeleton
        ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        t.options = ui
        import_tasks.append(t)
    else:
        print(f"Already imported: {asset_name}")

if import_tasks:
    print(f"Importing {len(import_tasks)} FBX animations with classic importer...")
    at.import_asset_tasks(import_tasks)
    print("Import finished!")

# 2. Retarget to player
rtg_path = f"{anim_dst_dir}/RTG_Pro_to_UEFN"
rtg = unreal.load_asset(rtg_path)
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
rig = unreal.load_asset(f"{anim_dst_dir}/IK_ProSwordShield")

if rtg and rig and ue_rig:
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
    print("RTG_Pro_to_UEFN configured!")

# Find AssetData for imported sequences
ar = unreal.AssetRegistryHelpers.get_asset_registry()
names_to_retarget = [asset_name for _, asset_name in key_anims]
assets_to_retarget = []
flt = unreal.ARFilter(package_paths=[anim_dst_dir], recursive_paths=True)
for a in ar.get_assets(flt):
    if str(a.asset_class_path.asset_name) == "AnimSequence" and str(a.asset_name) in names_to_retarget:
        out_pkg = f"{player_anims_dst}/RT_{a.asset_name}"
        if not eal.does_asset_exist(out_pkg):
            assets_to_retarget.append(a)

if assets_to_retarget and rtg:
    print(f"Retargeting {len(assets_to_retarget)} animations...")
    batch = unreal.IKRetargetBatchOperation()
    retargeted = batch.duplicate_and_retarget(
        assets_to_retarget, skel_mesh, ue_mesh, rtg, "", "", "RT_", "", player_anims_dst, False, False, True
    )
    for a in retargeted:
        eal.save_asset(str(a.package_name))
    print(f"Retargeted {len(retargeted)} player combat animations!")

# 3. Create Montages
def mk_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0):
    path = f"{player_montages_dst}/{montage_name}"
    seq = unreal.load_asset(f"{player_anims_dst}/{seq_name}")
    if not seq:
        print(f"Sequence {seq_name} not found for {montage_name}")
        return None
    if eal.does_asset_exist(path):
        m = unreal.load_asset(path)
    else:
        f = unreal.AnimMontageFactory()
        f.set_editor_property("source_animation", seq)
        m = at.create_asset(montage_name, player_montages_dst, unreal.AnimMontage, f)
    if m:
        b = m.get_editor_property("blend_in")
        b.set_editor_property("blend_time", bi)
        m.set_editor_property("blend_in", b)
        b = m.get_editor_property("blend_out")
        b.set_editor_property("blend_time", bo)
        m.set_editor_property("blend_out", b)
        m.set_editor_property("rate_scale", rate)
        eal.save_loaded_asset(m)
        print(f"Montage {montage_name} created/saved!")
    return m

l1 = mk_montage("RT_Pro_Slash_1", "AM_P_Pro_L1", 0.05, 0.15, 1.1)
l2 = mk_montage("RT_Pro_Slash_2", "AM_P_Pro_L2", 0.05, 0.15, 1.15)
l3 = mk_montage("RT_Pro_Slash_3", "AM_P_Pro_L3", 0.05, 0.15, 1.2)
l4 = mk_montage("RT_Pro_Slash_4", "AM_P_Pro_L4", 0.05, 0.15, 1.25)
l5 = mk_montage("RT_Pro_Slash_5", "AM_P_Pro_L5", 0.05, 0.25, 1.3)

h1 = mk_montage("RT_Pro_Attack_1", "AM_P_Pro_H1", 0.08, 0.2, 1.0)
h2 = mk_montage("RT_Pro_Attack_2", "AM_P_Pro_H2", 0.08, 0.2, 1.05)
h3 = mk_montage("RT_Pro_Kick", "AM_P_Pro_Kick", 0.08, 0.2, 1.1)
h4 = mk_montage("RT_Pro_PowerUp", "AM_P_Pro_PowerUp", 0.1, 0.25, 1.0)

blk = mk_montage("RT_Pro_Block", "AM_P_Pro_Block", 0.05, 0.1, 1.0)
imp = mk_montage("RT_Pro_Impact", "AM_P_Pro_Impact", 0.05, 0.1, 1.0)

out = {
    "montages": [
        "AM_P_Pro_L1", "AM_P_Pro_L2", "AM_P_Pro_L3", "AM_P_Pro_L4", "AM_P_Pro_L5",
        "AM_P_Pro_H1", "AM_P_Pro_H2", "AM_P_Pro_Kick", "AM_P_Pro_PowerUp",
        "AM_P_Pro_Block", "AM_P_Pro_Impact"
    ]
}

with open(f"{WS}/Saved/RRTools/import_pro_anims.json", "w") as f:
    json.dump(out, f, indent=2)

print("IMPORT_AND_RETARGET_PRO_ANIMS_COMPLETE")
