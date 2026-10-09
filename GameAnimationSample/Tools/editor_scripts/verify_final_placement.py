import unreal, json

out = {}

# 1. Load BP_RobotScout and DA_PlayerWeapon
bp_robot = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
weapon_cls = unreal.load_class(None, "/Script/AAACombat.AAAWeaponActor")
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
sword_mesh = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")

world = unreal.EditorLevelLibrary.get_editor_world()
robot = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_robot, unreal.Vector(0, 0, 100))

try:
    mesh = robot.get_component_by_class(unreal.SkeletalMeshComponent)
    weapon = unreal.EditorLevelLibrary.spawn_actor_from_class(weapon_cls, unreal.Vector(0, 0, 0))
    weapon.initialize_weapon(da)
    
    # Run the exact code AAACombatComponent runs:
    hand_socket = da.visuals.attachment_socket
    weapon.attach_to_component(mesh, hand_socket, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.KEEP_WORLD, False)
    weapon.set_actor_relative_transform(da.visuals.hand_offset, False, False)
    
    smc = weapon.get_component_by_class(unreal.StaticMeshComponent)
    
    # Hand location in world space
    hand_w = mesh.get_socket_location("hand_r")
    grip_w = smc.get_socket_location("Grip")
    pommel_w = smc.get_socket_location("") # or top
    trace_start_w = smc.get_socket_location("TraceStart")
    trace_end_w = smc.get_socket_location("TraceEnd")
    
    # Distance from hand_r wrist joint to Grip center
    dist_hand_grip = ((grip_w.x - hand_w.x)**2 + (grip_w.y - hand_w.y)**2 + (grip_w.z - hand_w.z)**2)**0.5
    
    # Vector from Grip to Tip (blade)
    blade_vec = trace_end_w - grip_w
    blade_len = blade_vec.length()
    blade_dir = blade_vec / max(blade_len, 0.001)
    
    # Robot axes
    rfwd = robot.get_actor_forward_vector()
    rright = robot.get_actor_right_vector()
    rup = robot.get_actor_up_vector()
    
    out = {
        "dist_hand_to_grip_center_cm": dist_hand_grip,
        "grip_world": [grip_w.x, grip_w.y, grip_w.z],
        "trace_start_world": [trace_start_w.x, trace_start_w.y, trace_start_w.z],
        "trace_end_tip_world": [trace_end_w.x, trace_end_w.y, trace_end_w.z],
        "blade_length_cm": blade_len,
        "blade_dir_relative_to_robot": {
            "forward": blade_dir.dot(rfwd),
            "right": blade_dir.dot(rright),
            "up": blade_dir.dot(rup)
        }
    }
    
    weapon.destroy_actor()
finally:
    robot.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/verify_final_placement.json", "w") as f:
    json.dump(out, f, indent=2)

print("VERIFY_FINAL_PLACEMENT_DONE")
