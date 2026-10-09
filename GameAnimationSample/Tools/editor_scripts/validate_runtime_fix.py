import unreal, json

out = {}

# 1. Check Sword Sockets
sword_a = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
grip_a = sword_a.find_socket("Grip") if sword_a else None
out["sword_a_grip"] = {
    "loc": [grip_a.relative_location.x, grip_a.relative_location.y, grip_a.relative_location.z] if grip_a else None,
    "rot": [grip_a.relative_rotation.pitch, grip_a.relative_rotation.yaw, grip_a.relative_rotation.roll] if grip_a else None
}

# 2. Check DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis = da.get_editor_property("visuals")
ho = vis.get_editor_property("hand_offset")
r = ho.rotation.rotator()
out["da_weapon"] = {
    "socket": str(vis.get_editor_property("attachment_socket")),
    "loc": [ho.translation.x, ho.translation.y, ho.translation.z],
    "rot": [r.pitch, r.yaw, r.roll]
}

# 3. Check RTG op
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)
num_ops = rc.get_num_retarget_ops()
ops = []
for i in range(num_ops):
    name = str(rc.get_op_name(i))
    ctrl = rc.get_op_controller(i)
    ops.append({
        "name": name,
        "class": ctrl.get_class().get_name() if ctrl else None
    })
out["rtg_ops"] = ops

# Test RetargetProfileLibrary controller fetch
prof = unreal.RetargetProfile()
res = unreal.RetargetProfileLibrary.get_op_controller_from_retarget_profile(prof, "Retarget IK Goals")
ctrl = res[0] if isinstance(res, tuple) else res
out["retarget_op_controller_valid"] = ctrl is not None

# 4. Spawn BP_RobotScout, spawn weapon actor, initialize with DA_PlayerWeapon and attach
world = unreal.EditorLevelLibrary.get_editor_world()
bp_robot = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
robot_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_robot, unreal.Vector(0, 0, 100))

try:
    mesh = robot_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    hand_loc = mesh.get_socket_location("hand_r")
    hand_t = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_WORLD)
    out["robot_hand_r_world_loc"] = [hand_loc.x, hand_loc.y, hand_loc.z]
    
    # Spawn weapon actor
    weapon_cls = unreal.load_class(None, "/Script/AAACombat.AAAWeaponActor")
    weapon_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(weapon_cls, unreal.Vector(0, 0, 0))
    
    try:
        weapon_actor.initialize_weapon(da)
        # Attach to hand_r
        weapon_actor.attach_to_component(mesh, "hand_r", unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.KEEP_WORLD, False)
        weapon_actor.set_actor_relative_transform(ho, False, False)
        
        # Weapon actor world transform
        w_loc = weapon_actor.get_actor_location()
        w_rot = weapon_actor.get_actor_rotation()
        out["weapon_actor_world_loc"] = [w_loc.x, w_loc.y, w_loc.z]
        out["weapon_actor_world_rot"] = [w_rot.pitch, w_rot.yaw, w_rot.roll]
        
        # Distance from hand_r to weapon actor
        dist = ((w_loc.x - hand_loc.x)**2 + (w_loc.y - hand_loc.y)**2 + (w_loc.z - hand_loc.z)**2)**0.5
        out["dist_hand_to_weapon"] = dist
        
        # StaticMeshComponent world location (where the sword mesh actually sits)
        smc = weapon_actor.get_component_by_class(unreal.StaticMeshComponent)
        if smc:
            smc_t = smc.get_world_transform()
            smc_loc = smc_t.translation
            out["sword_mesh_world_loc"] = [smc_loc.x, smc_loc.y, smc_loc.z]
            dist_mesh = ((smc_loc.x - hand_loc.x)**2 + (smc_loc.y - hand_loc.y)**2 + (smc_loc.z - hand_loc.z)**2)**0.5
            out["dist_hand_to_sword_mesh_origin"] = dist_mesh
            
            # Grip location in world space:
            # Grip socket on mesh is at (0, 0, 11) relative to smc
            # smc is shifted by -GripOffset = (0, 0, -11) relative to weapon actor
            # So Grip socket world location should be EXACTLY at weapon_actor_world_loc!
            grip_world_loc = smc.get_socket_location("Grip")
            out["grip_world_loc"] = [grip_world_loc.x, grip_world_loc.y, grip_world_loc.z]
            dist_grip_to_hand = ((grip_world_loc.x - hand_loc.x)**2 + (grip_world_loc.y - hand_loc.y)**2 + (grip_world_loc.z - hand_loc.z)**2)**0.5
            out["dist_grip_to_hand_r"] = dist_grip_to_hand
            
            # Blade tip location (Z=152 on mesh)
            tip_world_loc = smc.get_socket_location("") # or compute
            tip_dir = smc.get_up_vector() # Since sword length is along +Z in local space
            out["blade_direction_vector"] = [tip_dir.x, tip_dir.y, tip_dir.z]
    finally:
        weapon_actor.destroy_actor()
finally:
    robot_actor.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/validate_runtime_fix.json", "w") as f:
    json.dump(out, f, indent=2)

print("VALIDATE_RUNTIME_FIX_DONE")
