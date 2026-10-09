import unreal, json

AL = unreal.AnimationLibrary

src_dir = "/Game/RR/ExternalAnims/ProSwordShield"

stats = {}

for name in ["Pro_Slash_1", "Pro_Slash_2", "Pro_Slash_3", "Pro_Slash_4", "Pro_Slash_5", "Pro_Attack_1", "Pro_Attack_2"]:
    seq = unreal.load_asset(f"{src_dir}/{name}")
    if not seq:
        continue
    
    num_frames = AL.get_num_frames(seq)
    
    hips_z = []
    hips_pitch = []
    
    for f in range(num_frames):
        pose = AL.get_bone_pose_for_frame(seq, "Hips", f, False)
        hips_z.append(pose.translation.z)
        hips_pitch.append(pose.rotation.rotator().pitch)
        
    stats[name] = {
        "num_frames": num_frames,
        "length": AL.get_sequence_length(seq),
        "hips_z_min": min(hips_z),
        "hips_z_max": max(hips_z),
        "hips_z_avg": sum(hips_z) / len(hips_z),
        "hips_pitch_min": min(hips_pitch),
        "hips_pitch_max": max(hips_pitch),
        "hips_pitch_range": max(hips_pitch) - min(hips_pitch)
    }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/source_pro_bone_stats.json", "w") as f:
    json.dump(stats, f, indent=2)

print("SOURCE_STATS_DONE")
