import unreal, json

out = {}

# Load level L_RRArena
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
world = unreal.EditorLevelLibrary.get_editor_world()

# Find PlayerStart
acts = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
ps = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
p0 = ps[0].get_actor_location() if ps else unreal.Vector(0, 0, 100)
p0_rot = ps[0].get_actor_rotation() if ps else unreal.Rotator()

out["player_start"] = {
    "loc": [p0.x, p0.y, p0.z],
    "rot": [p0_rot.pitch, p0_rot.yaw, p0_rot.roll]
}

# Spawn BP_RobotScout at PlayerStart
bp_robot = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
robot_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_robot, p0)
robot_actor.set_actor_rotation(p0_rot, False)

try:
    mesh = robot_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    hand_t = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_WORLD)
    pelvis_t = mesh.get_socket_transform("pelvis", unreal.RelativeTransformSpace.RTS_WORLD)
    
    out["robot_forward"] = [robot_actor.get_actor_forward_vector().x, robot_actor.get_actor_forward_vector().y, robot_actor.get_actor_forward_vector().z]
    out["robot_right"] = [robot_actor.get_actor_right_vector().x, robot_actor.get_actor_right_vector().y, robot_actor.get_actor_right_vector().z]
    out["robot_up"] = [robot_actor.get_actor_up_vector().x, robot_actor.get_actor_up_vector().y, robot_actor.get_actor_up_vector().z]
    
    out["hand_r_world_loc"] = [hand_t.translation.x, hand_t.translation.y, hand_t.translation.z]
    r = hand_t.rotation.rotator()
    out["hand_r_world_rot"] = [r.pitch, r.yaw, r.roll]
    
    # Palm offset relative to hand_r:
    palm_off = unreal.Vector(-10.764, 5.811, -0.728)
    palm_world = hand_t.translation + hand_t.rotation.rotate_vector(palm_off)
    out["palm_world_loc"] = [palm_world.x, palm_world.y, palm_world.z]
    
    # In weapon space (since Grip socket is at Z=130):
    # Grip center is at (0, 0, 0)
    # Blade tip is at (0, 0, -130)
    # Pommel is at (0, 0, 22)
    # Crossguard is along (+-18.7, 0, 0)
    
    # Test rotations relative to hand_r:
    # We want:
    # In world space:
    # Blade should point:
    # 1. Forward/Down (pointing down towards the sand, slightly forward):
    #    world_dir = -0.3 * Up + 0.9 * Forward or similar (pointing down beside right leg)
    # 2. Or upright combat guard (blade pointing up/forward)
    
    test_rots = {
        "A_epic_original": unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0),
        "B_epic_flip_roll180": unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=180.0),
        "C_epic_flip_pitch180": unreal.Rotator(pitch=180.0 - 3.176, yaw=-180.0, roll=0.0),
        "D_pitch_0_yaw_0_roll_0": unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0),
        "E_pitch_0_yaw_180_roll_0": unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0),
        "F_pitch_90_yaw_0_roll_0": unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
        "G_pitch_neg90_yaw_0_roll_0": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
        "H_pitch_0_yaw_0_roll_90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=90.0),
        "I_pitch_0_yaw_0_roll_neg90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=-90.0),
        "J_pitch_90_yaw_180_roll_0": unreal.Rotator(pitch=90.0, yaw=180.0, roll=0.0),
        "K_pitch_neg90_yaw_180_roll_0": unreal.Rotator(pitch=-90.0, yaw=180.0, roll=0.0),
    }
    
    for name, rot in test_rots.items():
        w_rot = hand_t.rotation * rot.quaternion()
        tip_dir = w_rot.rotate_vector(unreal.Vector(0, 0, -1)) # blade tip direction
        pommel_dir = w_rot.rotate_vector(unreal.Vector(0, 0, 1))
        
        # Tip world location:
        tip_loc = palm_world + tip_dir * 130.0
        
        # Dot products with robot axes:
        fwd_dot = tip_dir.x * robot_actor.get_actor_forward_vector().x + tip_dir.y * robot_actor.get_actor_forward_vector().y + tip_dir.z * robot_actor.get_actor_forward_vector().z
        right_dot = tip_dir.x * robot_actor.get_actor_right_vector().x + tip_dir.y * robot_actor.get_actor_right_vector().y + tip_dir.z * robot_actor.get_actor_right_vector().z
        up_dot = tip_dir.x * robot_actor.get_actor_up_vector().x + tip_dir.y * robot_actor.get_actor_up_vector().y + tip_dir.z * robot_actor.get_actor_up_vector().z
        
        out[name] = {
            "rot": [rot.pitch, rot.yaw, rot.roll],
            "tip_loc_world": [tip_loc.x, tip_loc.y, tip_loc.z],
            "blade_dots_with_robot": {
                "forward": fwd_dot,
                "right": right_dot,
                "up": up_dot
            }
        }
finally:
    robot_actor.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_arena_sword_orient.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_ARENA_SWORD_ORIENT_DONE")
