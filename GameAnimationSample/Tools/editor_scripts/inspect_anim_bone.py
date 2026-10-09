import unreal, json

out = {}

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

try:
    bones = unreal.AnimationLibrary.get_bone_names(robot)
    out["num_bones"] = len(bones)
    out["has_hand_r"] = "hand_r" in [str(b) for b in bones]
except Exception as e:
    out["bones_err"] = str(e)

if seq:
    try:
        t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "hand_r", 0, True)
        out["hand_r_pose_0"] = {
            "loc": [t.translation.x, t.translation.y, t.translation.z],
            "rot": [t.rotation.rotator().pitch, t.rotation.rotator().yaw, t.rotation.rotator().roll]
        }
    except Exception as e:
        out["pose_err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_anim_bone.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ANIM_BONE_DONE")
