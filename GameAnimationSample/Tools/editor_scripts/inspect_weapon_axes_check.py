import unreal, json

out = {}

seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")
manny_skel = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/UE4_Mannequin_Skeleton")
robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")

# Check bone pose for hand_r in frame 0 and frame 10
for frame in [0, 5, 10, 20]:
    t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, "hand_r", frame, False)
    r = t.rotation.rotator()
    out[f"frame_{frame}_hand_r"] = {
        "loc": [t.translation.x, t.translation.y, t.translation.z],
        "rot": [r.pitch, r.yaw, r.roll]
    }

# Check weapon_r socket on UE4 Mannequin
sock_t = unreal.Transform(
    unreal.Vector(-10.764, 5.811, -0.728),
    unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0),
    unreal.Vector(1, 1, 1)
)

# For sword where length is along +Z:
# When sword is attached to weapon_r (relative transform = identity):
# Weapon direction in socket space is (0, 0, 1) (+Z)
# What is the blade direction in hand_r bone space?
# blade_in_hand = sock_rot * (0, 0, 1)
blade_in_hand = sock_t.rotation.rotate_vector(unreal.Vector(0, 0, 1))
out["blade_in_hand_using_epic_weapon_r"] = [blade_in_hand.x, blade_in_hand.y, blade_in_hand.z]

# And what if sword blade was along +X?
blade_x_in_hand = sock_t.rotation.rotate_vector(unreal.Vector(1, 0, 0))
out["blade_if_modeled_along_x"] = [blade_x_in_hand.x, blade_x_in_hand.y, blade_x_in_hand.z]

# And what if sword blade was along +Y?
blade_y_in_hand = sock_t.rotation.rotate_vector(unreal.Vector(0, 1, 0))
out["blade_if_modeled_along_y"] = [blade_y_in_hand.x, blade_y_in_hand.y, blade_y_in_hand.z]

# In hand_r:
# middle_01_r is at (-7.3, 0.2, 0.0) -> -X is towards the fingers!
# thumb_01_r is at (-1.3, 0.3, 2.5) -> +Z is towards the thumb!
# palm is towards +Y!
# When holding a sword in a normal forward grip:
# The blade should stick out towards -X (in direction of knuckles/punch) or +Z (out the top of the fist/thumb side) or -Z?
# When you make a fist:
# Top of the fist (thumb side) is +Z!
# Bottom of the fist (pinky side) is -Z!
# Knuckles (front of the fist) is -X!
# Palm (inside of the fist) is +Y!
# Back of the hand is -Y!

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_weapon_axes.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_WEAPON_AXES_DONE")
