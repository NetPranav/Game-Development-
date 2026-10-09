import unreal, json

eal = unreal.EditorAssetLibrary

# Test SkeletalMeshSocket renaming
robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
sock = robot_mesh.find_socket("weapon_r")
print(f"Existing weapon_r on robot: {sock}")
if not sock:
    try:
        new_sock = unreal.SkeletalMeshSocket(outer=robot_mesh)
        robot_mesh.add_socket(new_sock)
        print(f"Added sock default name: {new_sock.socket_name}")
        renamed = robot_mesh.rename_socket(new_sock.socket_name, "weapon_r")
        print(f"Renamed result: {renamed}")
        s = robot_mesh.find_socket("weapon_r")
        if s:
            s.set_editor_property("bone_name", "hand_r")
            s.set_editor_property("relative_location", unreal.Vector(-10.764, 5.811, -0.728))
            s.set_editor_property("relative_rotation", unreal.Rotator(pitch=0.0, yaw=-180.0, roll=0.0))
            eal.save_loaded_asset(robot_mesh)
            print("Successfully configured and saved weapon_r on SK_Robot_scout_R21!")
    except Exception as e:
        print(f"Error adding socket to skeletal mesh: {e}")

# Sockets on SM_LongSword
longsword = unreal.load_asset("/Game/RR/Weapons/Fab/long_sword/StaticMeshes/SM_LongSword")
if longsword:
    g = longsword.find_socket("Grip")
    if not g:
        g = unreal.StaticMeshSocket(outer=longsword)
        g.set_editor_property("socket_name", "Grip")
        longsword.add_socket(g)
    
    # Grip center X = -56.735
    # InvGrip will apply inverse of Grip relative transform.
    # In Grip, pitch = 90.0, yaw = 0.0, roll = 0.0 rotates blade (+X) to +Z!
    g.set_editor_property("relative_location", unreal.Vector(-56.735, 0.0, 0.0))
    g.set_editor_property("relative_rotation", unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0))
    
    t_start = longsword.find_socket("TraceStart")
    if not t_start:
        t_start = unreal.StaticMeshSocket(outer=longsword)
        t_start.set_editor_property("socket_name", "TraceStart")
        longsword.add_socket(t_start)
    t_start.set_editor_property("relative_location", unreal.Vector(-41.0, 0.0, 0.0))
    
    t_end = longsword.find_socket("TraceEnd")
    if not t_end:
        t_end = unreal.StaticMeshSocket(outer=longsword)
        t_end.set_editor_property("socket_name", "TraceEnd")
        longsword.add_socket(t_end)
    t_end.set_editor_property("relative_location", unreal.Vector(75.0, 0.0, 0.0))
    
    eal.save_loaded_asset(longsword)
    print("Saved sockets on SM_LongSword")

# Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    vis = da.get_editor_property("visuals")
    vis.set_editor_property("attachment_socket", "weapon_r")
    vis.set_editor_property("hand_offset", unreal.Transform(unreal.Vector(0,0,0), unreal.Rotator(0,0,0), unreal.Vector(1,1,1)))
    da.set_editor_property("visuals", vis)
    eal.save_loaded_asset(da)
    print("Saved DA_PlayerWeapon")

# Setup L_RRArena
LEVEL = "/Game/RR/Levels/L_RRArena"
unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

# Find Colosseum
col_actor = None
for a in acts:
    if "colosseum" in a.get_actor_label().lower():
        col_actor = a
        break
    smc = a.get_component_by_class(unreal.StaticMeshComponent)
    if smc and smc.static_mesh and "colosseum" in smc.static_mesh.get_name().lower():
        col_actor = a
        break

if not col_actor:
    for a in acts:
        if a.get_actor_label() == "StaticMeshActor_47":
            col_actor = a
            break

if col_actor:
    c_loc = col_actor.get_actor_location()
    CX, CY, CZ = c_loc.x, c_loc.y, c_loc.z
    print(f"Colosseum at ({CX:.1f}, {CY:.1f}, {CZ:.1f})")

    def trace_ground(x, y, z_start=6000.0, z_end=-6000.0):
        hit = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(x, y, z_start), unreal.Vector(x, y, z_end),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
        )
        if hit:
            t = hit.to_tuple()
            return t[5].z, t[9]
        return None, None

    floor_z, _ = trace_ground(CX, CY)
    if floor_z is None:
        floor_z = CZ
    print(f"Arena floor at Z={floor_z}")

    arenas = [a for a in acts if a.get_class().get_name() == "RRArena"]
    if not arenas:
        cls = unreal.load_class(None, "/Script/RRGame.RRArena")
        arena = eas.spawn_actor_from_class(cls, unreal.Vector(CX, CY, floor_z), unreal.Rotator(0, 0, 0))
        arena.set_actor_label("RR_DuelArena")
    else:
        arena = arenas[0]
        arena.set_actor_location(unreal.Vector(CX, CY, floor_z), False, True)

    # Set Radius = 1750 (mid part green combat arena!)
    arena.set_editor_property("radius", 1750.0)
    arena.set_editor_property("entry_radius", 250.0)
    arena.set_editor_property("entry_hold_seconds", 0.5)

    def set_rel(c_name, rx, ry, rz):
        try:
            c = arena.get_editor_property(c_name)
            c.set_editor_property("relative_location", unreal.Vector(rx, ry, rz))
        except Exception as ex:
            print(f"Failed to set {c_name}: {ex}")

    set_rel("player_spot", 0.0, -600.0, 50.0)
    set_rel("boss_spot", 0.0, 600.0, 50.0)
    set_rel("boss_entrance", 0.0, 1100.0, 50.0)

    # PlayerStart outside on top of stairs / path leading to arena:
    # Outer walkway/stairs: (3457.8, -1170.0, 55.4)
    # Direction towards arena center (CX, CY):
    ps_actors = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
    if ps_actors:
        ps = ps_actors[0]
        # Position PlayerStart outside on the walkway at top of entrance stairs
        ps_x, ps_y = 3450.0, -1170.0
        ps_z, _ = trace_ground(ps_x, ps_y)
        if ps_z is None: ps_z = 55.4
        ps.set_actor_location(unreal.Vector(ps_x, ps_y, ps_z + 92.0), False, True)
        
        # Face towards colosseum center
        import math
        face_yaw = math.degrees(math.atan2(CY - ps_y, CX - ps_x))
        ps.set_actor_rotation(unreal.Rotator(0, face_yaw, 0), False)
        print(f"PlayerStart placed outside at ({ps_x}, {ps_y}, {ps_z + 92.0}) facing Yaw={face_yaw:.1f}")

        # EntryPoint field effect at the boundary entering the green arena:
        # Distance 1750 from center along direction towards PlayerStart
        dx = ps_x - CX
        dy = ps_y - CY
        dist = math.hypot(dx, dy)
        ux = dx / dist if dist > 0.0 else 1.0
        uy = dy / dist if dist > 0.0 else 0.0
        
        entry_x = CX + ux * 1750.0
        entry_y = CY + uy * 1750.0
        ez, _ = trace_ground(entry_x, entry_y)
        entry_z = (ez + 20.0) if ez is not None else (floor_z + 20.0)
        
        try:
            ep = arena.get_editor_property("entry_point")
            ep.set_world_location(unreal.Vector(entry_x, entry_y, entry_z))
            xp = arena.get_editor_property("exit_point")
            xp.set_world_location(unreal.Vector(entry_x + ux * 150.0, entry_y + uy * 150.0, entry_z))
            print(f"EntryPoint placed at boundary: ({entry_x:.1f}, {entry_y:.1f}, {entry_z:.1f})")
        except Exception as ex:
            print(f"Failed to set entry_point: {ex}")

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    print("SUCCESS: L_RRArena saved successfully!")

print("TEST_FIX_AND_SETUP_COMPLETE")
