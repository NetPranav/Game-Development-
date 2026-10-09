import unreal, json

AL = unreal.AnimationLibrary

seq = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/Pro_Block")
num_f = AL.get_num_frames(seq)

# Let's inspect left hand (shield) and right hand (sword) and root/pelvis across frames
frames_data = []
for f in range(num_f):
    lh = AL.get_bone_pose_for_frame(seq, "LeftHand", f, False)
    rh = AL.get_bone_pose_for_frame(seq, "RightHand", f, False)
    frames_data.append({
        "frame": f,
        "lh_loc": [lh.translation.x, lh.translation.y, lh.translation.z],
        "rh_loc": [rh.translation.x, rh.translation.y, rh.translation.z]
    })

# Also check other block animations in the project:
# UAL2_StandardSword_Block, UAL2_StandardIdle_Shield_Loop
extra = {}
for extra_name in [
    "/Game/RR/ExternalAnims/UAL2/UAL2_Standard/SkeletalMeshes/UAL2_StandardIdle_Shield_Loop",
    "/Game/RR/ExternalAnims/UAL2/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Block"
]:
    es = unreal.load_asset(extra_name)
    if es:
        extra[extra_name] = {
            "num_frames": AL.get_num_frames(es),
            "length": AL.get_sequence_length(es)
        }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/block_frames_detail.json", "w") as f:
    json.dump({"pro_block_frames": frames_data, "extra": extra}, f, indent=2)

print("BLOCK_FRAMES_DONE")
