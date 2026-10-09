import unreal, json

seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

# Let's check root and head_01 in frame 0
head_t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "head", 0, False)
pelvis_t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "pelvis", 0, False)
foot_r = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "foot_r", 0, False)
foot_l = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "foot_l", 0, False)

out = {
    "pelvis": [pelvis_t.translation.x, pelvis_t.translation.y, pelvis_t.translation.z],
    "foot_r": [foot_r.translation.x, foot_r.translation.y, foot_r.translation.z],
    "foot_l": [foot_l.translation.x, foot_l.translation.y, foot_l.translation.z],
}

with open(unreal.Paths.project_saved_dir() + "RRTools/char_axes.json", "w") as f:
    json.dump(out, f, indent=2)

print("CHAR_AXES_DONE")
