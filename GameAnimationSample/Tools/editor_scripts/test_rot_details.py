import unreal, json

# We know the hand_r transform in idle pose from inspect_finger_poses.json:
# In idle pose (frame 0 of RT_UAL2_StandardSword_Regular_A or ref pose):
# Let's inspect the world transform of hand_r in the actual character blueprint or animation
seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

# Let's check bone pose of hand_r, lowerarm_r, upperarm_r
for b in ["hand_r", "lowerarm_r", "clavicle_r"]:
    t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, b, 0, False)
    print(f"{b}: loc={t.translation}, rot={t.rotation.rotator()}")

# Let's test candidate HandOffset transforms
# Epic weapon_r: loc=(-10.764, 5.811, -0.728), rot=Rotator(pitch=-3.176, yaw=-180.0, roll=0.0)
epic_rot = unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0)
epic_loc = unreal.Vector(-10.764, 5.811, -0.728)

print("Epic rot rotator:", epic_rot)
print("Epic loc vector:", epic_loc)
