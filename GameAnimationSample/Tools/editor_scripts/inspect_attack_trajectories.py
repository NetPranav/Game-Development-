import unreal, json

AL = unreal.AnimationLibrary

src_dir = "/Game/RR/ExternalAnims/ProSwordShield"

out = {}
for name in ["Pro_Slash_1", "Pro_Slash_3", "Pro_Slash_5", "Pro_Attack_2", "Pro_Block", "Pro_Impact"]:
    seq = unreal.load_asset(f"{src_dir}/{name}")
    if not seq:
        continue
    # Sample right hand trajectory
    n = AL.get_num_frames(seq)
    rh_zs = []
    rh_xs = []
    rh_ys = []
    for f in range(n):
        pose = AL.get_bone_pose_for_frame(seq, "RightHand", f, False)
        rh_zs.append(pose.translation.z)
        rh_xs.append(pose.translation.x)
        rh_ys.append(pose.translation.y)
    out[name] = {
        "len": AL.get_sequence_length(seq),
        "rh_z_range": [min(rh_zs), max(rh_zs)],
        "rh_x_range": [min(rh_xs), max(rh_xs)],
        "rh_y_range": [min(rh_ys), max(rh_ys)]
    }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/attack_types.json", "w") as f:
    json.dump(out, f, indent=2)

print("ATTACK_TYPES_DONE")
