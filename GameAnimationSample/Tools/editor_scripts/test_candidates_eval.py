import unreal, json

# Let's inspect the exact transform of the sword mesh on BP_RobotScout in L_RRArena
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
world = unreal.EditorLevelLibrary.get_editor_world()

da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
sword_mesh = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")

# Verify Grip socket on SM_Sword_A
grip_sock = sword_mesh.find_socket("Grip")
grip_loc = grip_sock.relative_location if grip_sock else None

bp_robot = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
weapon_cls = unreal.load_class(None, "/Script/AAACombat.AAAWeaponActor")

# Spawn robot at (0, 0, 100)
robot = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_robot, unreal.Vector(0, 0, 100))

out = {
    "grip_socket_loc": [grip_loc.x, grip_loc.y, grip_loc.z] if grip_loc else None
}

# Test candidate HandOffset transforms
# Palm offset: (-10.764, 5.811, -0.728)
candidates = {
    "roll_neg90": unreal.Transform(unreal.Vector(-10.764, 5.811, -0.728), unreal.Rotator(pitch=0.0, yaw=0.0, roll=-90.0), unreal.Vector(1, 1, 1)),
    "pitch_neg90": unreal.Transform(unreal.Vector(-10.764, 5.811, -0.728), unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0), unreal.Vector(1, 1, 1)),
    "roll_neg90_yaw_neg90": unreal.Transform(unreal.Vector(-10.764, 5.811, -0.728), unreal.Rotator(pitch=0.0, yaw=-90.0, roll=-90.0), unreal.Vector(1, 1, 1)),
    "roll_neg90_pitch_10": unreal.Transform(unreal.Vector(-10.764, 5.811, -0.728), unreal.Rotator(pitch=10.0, yaw=0.0, roll=-90.0), unreal.Vector(1, 1, 1)),
    "roll_neg90_pitch_neg10": unreal.Transform(unreal.Vector(-10.764, 5.811, -0.728), unreal.Rotator(pitch=-10.0, yaw=0.0, roll=-90.0), unreal.Vector(1, 1, 1)),
}

try:
    mesh = robot.get_component_by_class(unreal.SkeletalMeshComponent)
    weapon = unreal.EditorLevelLibrary.spawn_actor_from_class(weapon_cls, unreal.Vector(0, 0, 0))
    weapon.initialize_weapon(da)
    weapon.attach_to_component(mesh, "hand_r", unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.KEEP_WORLD, False)
    
    smc = weapon.get_component_by_class(unreal.StaticMeshComponent)
    
    for c_name, t in candidates.items():
        weapon.set_actor_relative_transform(t, False, False)
        
        # In world space:
        grip_w = smc.get_socket_location("Grip")
        trace_start_w = smc.get_socket_location("TraceStart") # base of blade
        trace_end_w = smc.get_socket_location("TraceEnd") # tip of blade
        
        # Also check actor forward, right, up
        act_fwd = robot.get_actor_forward_vector()
        act_right = robot.get_actor_right_vector()
        act_up = robot.get_actor_up_vector()
        
        blade_vec = trace_end_w - grip_w
        blade_len = blade_vec.length()
        blade_dir = blade_vec / max(blade_len, 0.001)
        
        out[c_name] = {
            "grip_world": [grip_w.x, grip_w.y, grip_w.z],
            "tip_world": [trace_end_w.x, trace_end_w.y, trace_end_w.z],
            "blade_dir_relative_to_robot": {
                "forward": blade_dir.dot(act_fwd),
                "right": blade_dir.dot(act_right),
                "up": blade_dir.dot(act_up),
            }
        }
    weapon.destroy_actor()
finally:
    robot.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_candidates_eval.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_CANDIDATES_EVAL_DONE")
