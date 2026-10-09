import unreal, json

# 1. First, set Grip socket on SM_Sword_A and SM_Sword_B to Z = 130.0
eal = unreal.EditorAssetLibrary
for path in ["/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A", "/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B"]:
    mesh = unreal.load_asset(path)
    s = mesh.find_socket("Grip")
    if s:
        s.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 130.0))
        s.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
        eal.save_loaded_asset(mesh)

# 2. Check candidate rotations
# In hand_r:
# hand_r center of palm is at (-10.764, 5.811, -0.728) or similar
# Let's test candidate transforms for DA_PlayerWeapon
# In weapon space:
# Grip is at (0,0,0) (because SMC is shifted by -GripOffset = (0,0,-130))
# Blade tip is at (0, 0, -130) relative to weapon actor!
# Pommel is at (0, 0, +22) relative to weapon actor!

rotations = {
    # If rot = (0, 180, 0): Pitch=180
    "pitch_180": unreal.Rotator(pitch=180.0, yaw=0.0, roll=0.0),
    # If rot = (0, 0, 180): Roll=180
    "roll_180": unreal.Rotator(pitch=0.0, yaw=0.0, roll=180.0),
    # If rot = (180, 0, 0): Yaw=180
    "yaw_180": unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0),
    # Epic weapon_r standard was: pitch=-3.176, yaw=-180, roll=0.
    # But that was when weapon was assumed to have blade along +Z!
    # With blade along -Z, adding 180 pitch:
    "epic_flipped_pitch": unreal.Rotator(pitch=180.0 - 3.176, yaw=-180.0, roll=0.0),
    "epic_flipped_roll": unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=180.0),
    "epic_original": unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0),
    # What if pitch=90 or -90?
    "pitch_90": unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
    "pitch_neg90": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
    "yaw_90": unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0),
    "yaw_neg90": unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0),
}

# Let's test on robot character
bp_robot = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
robot_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_robot, unreal.Vector(0, 0, 100))

out = {}
try:
    mesh = robot_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    hand_t = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_WORLD)
    
    # Palm location: hand_t translated by (-10.764, 5.811, -0.728) in hand space
    palm_off = unreal.Vector(-10.764, 5.811, -0.728)
    
    for name, r in rotations.items():
        # Weapon actor world transform:
        w_rot = hand_t.rotation * r.quaternion()
        w_loc = hand_t.translation + hand_t.rotation.rotate_vector(palm_off)
        
        # In weapon space:
        # Grip = (0, 0, 0) -> world loc is w_loc
        # Blade Tip = (0, 0, -130) -> vector from grip to tip is w_rot * (0, 0, -130)
        tip_dir = w_rot.rotate_vector(unreal.Vector(0, 0, -1)) # Unit direction of blade
        pommel_dir = w_rot.rotate_vector(unreal.Vector(0, 0, 1))
        crossguard_dir = w_rot.rotate_vector(unreal.Vector(1, 0, 0))
        
        out[name] = {
            "rot": [r.pitch, r.yaw, r.roll],
            "blade_dir_world": [tip_dir.x, tip_dir.y, tip_dir.z],
            "pommel_dir_world": [pommel_dir.x, pommel_dir.y, pommel_dir.z],
            "crossguard_dir_world": [crossguard_dir.x, crossguard_dir.y, crossguard_dir.z]
        }
finally:
    robot_actor.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_sword_rotations.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_SWORD_ROTATIONS_DONE")
