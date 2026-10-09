import unreal, json

eal = unreal.EditorAssetLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
AL = unreal.AnimationLibrary

anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"

rtg = unreal.load_asset(f"{anim_dst_dir}/RTG_Pro_to_UEFN")
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")

batch = unreal.IKRetargetBatchOperation()

tgt_path = f"{player_anims_dst}/RT_Pro_Slash_1"
if eal.does_asset_exist(tgt_path):
    eal.delete_asset(tgt_path)

src_path = f"{anim_dst_dir}/Pro_Slash_1.Pro_Slash_1"
data = ar.get_asset_by_object_path(src_path)

retargeted = batch.duplicate_and_retarget(
    [data], skel_mesh, ue_mesh, rtg, "", "", "RT_", "", player_anims_dst, False, False, True
)

res = {}
if retargeted:
    first = retargeted[0]
    cur_pkg = str(first.package_name)
    if cur_pkg != tgt_path:
        eal.rename_asset(cur_pkg, tgt_path)
        eal.save_asset(tgt_path)
    else:
        eal.save_asset(cur_pkg)
        
    seq = unreal.load_asset(tgt_path)
    num_f = AL.get_num_frames(seq)
    
    pelvis_zs = []
    pelvis_pitches = []
    root_locs = []
    root_rots = []
    
    for f in range(0, num_f, max(1, num_f // 10)):
        p_pose = AL.get_bone_pose_for_frame(seq, "pelvis", f, False)
        r_pose = AL.get_bone_pose_for_frame(seq, "root", f, False)
        pelvis_zs.append(p_pose.translation.z)
        pelvis_pitches.append(p_pose.rotation.rotator().pitch)
        root_locs.append([r_pose.translation.x, r_pose.translation.y, r_pose.translation.z])
        root_rots.append([r_pose.rotation.rotator().pitch, r_pose.rotation.rotator().yaw, r_pose.rotation.rotator().roll])
        
    res["status"] = "Success"
    res["pelvis_z_min"] = min(pelvis_zs)
    res["pelvis_z_max"] = max(pelvis_zs)
    res["pelvis_pitch_min"] = min(pelvis_pitches)
    res["pelvis_pitch_max"] = max(pelvis_pitches)
    res["root_sample_locs"] = root_locs
    res["root_sample_rots"] = root_rots
else:
    res["status"] = "Failed"

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/test_clean_retarget_slash1.json", "w") as f:
    json.dump(res, f, indent=2)

print("TEST_RETARGET_DONE")
