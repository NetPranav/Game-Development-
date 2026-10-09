import unreal, json

AL = unreal.AnimationLibrary
eal = unreal.EditorAssetLibrary

src_dir = "/Game/RR/ExternalAnims/ProSwordShield"

out = {}
for asset in ["Pro_Slash_1", "Pro_Slash_2", "Pro_Slash_3", "Pro_Slash_4", "Pro_Slash_5", "Pro_Attack_1", "Pro_Attack_2"]:
    seq = unreal.load_asset(f"{src_dir}/{asset}")
    if not seq:
        continue
    n = AL.get_num_frames(seq)
    len_sec = AL.get_sequence_length(seq)
    
    # Check max and min Z of hips or root to detect jumping
    # Also check forward translation X/Y
    hip_zs = []
    hip_xs = []
    hip_ys = []
    for f in range(0, n, max(1, n//20)):
        pose = AL.get_bone_pose_for_frame(seq, "Hips", f, False)
        hip_zs.append(pose.translation.z)
        hip_xs.append(pose.translation.x)
        hip_ys.append(pose.translation.y)
        
    out[asset] = {
        "frames": n,
        "length": len_sec,
        "hip_z_min": min(hip_zs),
        "hip_z_max": max(hip_zs),
        "hip_z_range": max(hip_zs) - min(hip_zs),
        "hip_x_range": max(hip_xs) - min(hip_xs),
        "hip_y_range": max(hip_ys) - min(hip_ys)
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/slash_stats.json", "w") as f:
    json.dump(out, f, indent=2)

print("SLASH_STATS_DONE")
