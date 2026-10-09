import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

out = {}
bones_to_check = ["hand_r", "index_01_r", "thumb_01_r", "pinky_01_r", "middle_01_r"]

for b in bones_to_check:
    try:
        t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, b, 0, False)
        out[b] = {
            "loc": [t.translation.x, t.translation.y, t.translation.z],
            "rot": [t.rotation.rotator().pitch, t.rotation.rotator().yaw, t.rotation.rotator().roll]
        }
    except Exception as e:
        out[b + "_err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_finger_poses.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_FINGER_POSES_DONE")
