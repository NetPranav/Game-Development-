import unreal, json

montage = unreal.load_asset("/Game/RR/Player/AM_P_UAL_L1")
seq_asset = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

out = {}

# Sockets and candidates to test:
# Candidate 1: roll = -90 (Rotator(pitch=0, yaw=0, roll=-90))
# Candidate 2: roll = 90
# Candidate 3: pitch = 90, roll = -90
# Candidate 4: pitch = -90
candidates = {
    "roll_neg90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=-90.0),
    "roll_pos90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=90.0),
    "pitch_90": unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
    "pitch_neg90": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
    "pitch_90_roll_neg90": unreal.Rotator(pitch=90.0, yaw=0.0, roll=-90.0),
    "pitch_neg90_roll_neg90": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=-90.0),
    "yaw_neg90_roll_neg90": unreal.Rotator(pitch=0.0, yaw=-90.0, roll=-90.0),
    "yaw_90_roll_neg90": unreal.Rotator(pitch=0.0, yaw=90.0, roll=-90.0),
}

# For frame 0 (start), frame 15 (swing apex), frame 30 (slash impact):
frames = [0, 10, 20, 30]

for c_name, rot in candidates.items():
    frame_data = {}
    for f in frames:
        t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq_asset, "hand_r", f, False)
        # In hand space:
        # Blade direction is rot * (0, 0, -1)
        w_rot = t.rotation * rot.quaternion()
        tip_dir = w_rot.rotate_vector(unreal.Vector(0, 0, -1))
        frame_data[f"frame_{f}"] = [tip_dir.x, tip_dir.y, tip_dir.z]
    out[c_name] = frame_data

with open(unreal.Paths.project_saved_dir() + "RRTools/attack_sword_orient.json", "w") as f:
    json.dump(out, f, indent=2)

print("ATTACK_SWORD_ORIENT_DONE")
