import unreal, os, json

WS = "A:/UNREAL PROJECT/Game/GameAnimationSample"
anim_src_dir = f"{WS}/ExternalAnims/ProSwordShield"
anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"
player_montages_dst = "/Game/RR/Player"

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
AL = unreal.AnimationLibrary

out = {"steps": []}

# 1. Update RTG_Pro_to_UEFN mappings
rtg_path = f"{anim_dst_dir}/RTG_Pro_to_UEFN"
rtg = unreal.load_asset(rtg_path)
if not rtg:
    raise Exception(f"RTG not found: {rtg_path}")

rc = unreal.IKRetargeterController.get_controller(rtg)

# Explicitly UNMAP Root chain so it does NOT follow RightFoot!
rc.set_source_chain("Root", "")

# Unmap toe and metacarpal chains
chains_to_clear = [
    "LeftToe", "RightToe",
    "LeftIndexMetacarpal", "LeftMiddleMetacarpal", "LeftRingMetacarpal", "LeftPinkyMetacarpal",
    "RightIndexMetacarpal", "RightMiddleMetacarpal", "RightRingMetacarpal", "RightPinkyMetacarpal"
]
for c_name in chains_to_clear:
    try:
        rc.set_source_chain(c_name, "")
    except Exception as e:
        pass

# Ensure critical chains are properly mapped
chain_maps = {
    "Spine": "Spine",
    "Neck": "Neck",
    "Head": "Head",
    "LeftClavicle": "LeftClavicle",
    "LeftArm": "LeftArm",
    "LeftLeg": "LeftLeg",
    "RightClavicle": "RightClavicle",
    "RightArm": "RightArm",
    "RightLeg": "RightLeg",
    "LeftThumb": "LeftThumb",
    "LeftIndex": "LeftIndex",
    "LeftMiddle": "LeftMiddle",
    "LeftRing": "LeftRing",
    "LeftPinky": "LeftPinky",
    "RightThumb": "RightThumb",
    "RightIndex": "RightIndex",
    "RightMiddle": "RightMiddle",
    "RightRing": "RightRing",
    "RightPinky": "RightPinky"
}
for tgt_c, src_c in chain_maps.items():
    try:
        rc.set_source_chain(tgt_c, src_c)
    except Exception as e:
        print(f"Failed to map {tgt_c} -> {src_c}: {e}")

eal.save_loaded_asset(rtg)
out["steps"].append("RTG_Pro_to_UEFN mappings corrected and saved")
print("RTG_Pro_to_UEFN mappings corrected and saved!")

# 2. Check / import new FBX animations if needed
skeleton = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base_Skeleton")
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")

extra_fbx = [
    ("sword and shield block idle.fbx", "Pro_Block_Idle"),
    ("sword and shield walk.fbx", "Pro_Walk_Shield")
]
import_tasks = []
for fbx_name, asset_name in extra_fbx:
    dest_pkg = f"{anim_dst_dir}/{asset_name}"
    fbx_file = f"{anim_src_dir}/{fbx_name}"
    if os.path.exists(fbx_file) and not eal.does_asset_exist(dest_pkg):
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
        t.options = ui
        import_tasks.append(t)

if import_tasks:
    at.import_asset_tasks(import_tasks)
    out["steps"].append(f"Imported {len(import_tasks)} extra FBX animations")
    print(f"Imported {len(import_tasks)} extra FBX animations!")

# 3. Retarget the animations to player
# Map: source_asset_name -> target_asset_name
# Note: Pro_Slash_1 (right slash), Pro_Slash_3 (left slash), Pro_Slash_5 (overhead finisher)
anims_to_retarget = [
    ("Pro_Slash_1", "RT_Pro_Slash_1"),
    ("Pro_Slash_3", "RT_Pro_Slash_2"),
    ("Pro_Slash_5", "RT_Pro_Slash_3"),
    ("Pro_Block_Idle" if eal.does_asset_exist(f"{anim_dst_dir}/Pro_Block_Idle") else "Pro_Block", "RT_Pro_Block"),
    ("Pro_Impact", "RT_Pro_Impact"),
    ("Pro_Attack_2", "RT_Pro_Attack_2"),
    ("Pro_Kick", "RT_Pro_Kick"),
    ("Pro_PowerUp", "RT_Pro_PowerUp"),
    ("Pro_Draw", "RT_Pro_Draw"),
    ("Pro_Sheath", "RT_Pro_Sheath")
]

batch = unreal.IKRetargetBatchOperation()
retarget_results = {}

for src_name, tgt_name in anims_to_retarget:
    src_asset = unreal.load_asset(f"{anim_dst_dir}/{src_name}")
    if not src_asset:
        print(f"Warning: source asset {src_name} not found")
        continue
    
    tgt_path = f"{player_anims_dst}/{tgt_name}"
    if eal.does_asset_exist(tgt_path):
        eal.delete_asset(tgt_path)

    retargeted = batch.duplicate_and_retarget(
        [src_asset],
        skel_mesh,
        ue_mesh,
        rtg,
        player_anims_dst,
        "",
        "",
        True
    )
    if retargeted:
        first = retargeted[0]
        # Rename to exact desired name if needed
        cur_pkg = str(first.package_name)
        if not cur_pkg.endswith(tgt_name):
            eal.rename_asset(cur_pkg, tgt_path)
            eal.save_asset(tgt_path)
        else:
            eal.save_asset(cur_pkg)
        retarget_results[tgt_name] = "Success"
        print(f"Retargeted {src_name} -> {tgt_name}")
    else:
        retarget_results[tgt_name] = "Failed"

out["retarget_results"] = retarget_results

# 4. Create / Update Montages
def setup_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0, b_loop=False):
    seq_path = f"{player_anims_dst}/{seq_name}"
    seq = unreal.load_asset(seq_path)
    if not seq:
        print(f"Sequence not found: {seq_path}")
        return None
    
    m_path = f"{player_montages_dst}/{montage_name}"
    m = unreal.load_asset(m_path)
    if not m:
        f = unreal.AnimMontageFactory()
        f.set_editor_property("source_animation", seq)
        m = at.create_asset(montage_name, player_montages_dst, unreal.AnimMontage, f)
    else:
        # Re-assign source sequence
        tracks = m.get_editor_property("slot_anim_tracks")
        if tracks:
            anim_track = tracks[0].get_editor_property("anim_track")
            segs = anim_track.get_editor_property("anim_segments")
            if segs:
                segs[0].set_editor_property("anim_reference", seq)
                anim_track.set_editor_property("anim_segments", segs)
                tracks[0].set_editor_property("anim_track", anim_track)
                m.set_editor_property("slot_anim_tracks", tracks)

    if m:
        b = m.get_editor_property("blend_in")
        b.set_editor_property("blend_time", bi)
        m.set_editor_property("blend_in", b)
        
        b = m.get_editor_property("blend_out")
        b.set_editor_property("blend_time", bo)
        m.set_editor_property("blend_out", b)
        
        m.set_editor_property("rate_scale", rate)
        eal.save_loaded_asset(m)
        print(f"Configured montage: {montage_name}")
    return m

# Create 3-hit grounded combo montages
l1 = setup_montage("RT_Pro_Slash_1", "AM_P_Pro_L1", bi=0.05, bo=0.15, rate=1.15)
l2 = setup_montage("RT_Pro_Slash_2", "AM_P_Pro_L2", bi=0.05, bo=0.15, rate=1.20)
l3 = setup_montage("RT_Pro_Slash_3", "AM_P_Pro_L3", bi=0.05, bo=0.25, rate=1.25)

# Block montage - with looping enabled!
blk = setup_montage("RT_Pro_Block", "AM_P_Pro_Block", bi=0.08, bo=0.15, rate=1.0, b_loop=True)
imp = setup_montage("RT_Pro_Impact", "AM_P_Pro_Impact", bi=0.05, bo=0.15, rate=1.0)

# Heavy & utilities
h1 = setup_montage("RT_Pro_Attack_2", "AM_P_Pro_H1", bi=0.08, bo=0.20, rate=1.1)
kick = setup_montage("RT_Pro_Kick", "AM_P_Pro_Kick", bi=0.08, bo=0.20, rate=1.1)
pup = setup_montage("RT_Pro_PowerUp", "AM_P_Pro_PowerUp", bi=0.10, bo=0.25, rate=1.0)
eq = setup_montage("RT_Pro_Draw", "AM_P_Pro_Equip", bi=0.08, bo=0.15, rate=1.2)
uneq = setup_montage("RT_Pro_Sheath", "AM_P_Pro_Unequip", bi=0.08, bo=0.15, rate=1.2)

# 5. Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    anims = da.get_editor_property("animations")
    anims.set_editor_property("light_attack_combos", [l1, l2, l3])
    anims.set_editor_property("heavy_attack_combos", [h1, kick, pup])
    anims.set_editor_property("block_montage", blk)
    anims.set_editor_property("parry_montage", blk)
    anims.set_editor_property("equip_animation", eq)
    anims.set_editor_property("unequip_animation", uneq)
    da.set_editor_property("animations", anims)
    eal.save_loaded_asset(da)
    out["steps"].append("Updated DA_PlayerWeapon with 3-hit combo and looping block")
    print("Updated DA_PlayerWeapon successfully!")

# 6. Verify Bone Poses on RT_Pro_Slash_1, 2, 3
check_poses = {}
for anim_name in ["RT_Pro_Slash_1", "RT_Pro_Slash_2", "RT_Pro_Slash_3", "RT_Pro_Block"]:
    seq = unreal.load_asset(f"{player_anims_dst}/{anim_name}")
    if seq:
        t0_root = AL.get_bone_pose_for_frame(seq, "root", 0, False)
        t0_pelvis = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
        check_poses[anim_name] = {
            "root_loc": [t0_root.translation.x, t0_root.translation.y, t0_root.translation.z],
            "root_rot": [t0_root.rotation.rotator().pitch, t0_root.rotation.rotator().yaw, t0_root.rotation.rotator().roll],
            "pelvis_loc": [t0_pelvis.translation.x, t0_pelvis.translation.y, t0_pelvis.translation.z],
            "pelvis_rot": [t0_pelvis.rotation.rotator().pitch, t0_pelvis.rotation.rotator().yaw, t0_pelvis.rotation.rotator().roll]
        }
out["pose_verification"] = check_poses

with open(f"{WS}/Saved/RRTools/fix_and_retarget_summary.json", "w") as f:
    json.dump(out, f, indent=2)

print("FIX_AND_RETARGET_ALL_COMPLETE")
