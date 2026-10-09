import unreal, json

eal = unreal.EditorAssetLibrary

# 1. Add weapon_r socket to SK_Robot_scout_R21
robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
if robot_mesh:
    s = robot_mesh.find_socket("weapon_r")
    if not s:
        s = unreal.SkeletalMeshSocket(outer=robot_mesh)
        s.set_editor_property("socket_name", "weapon_r")
        robot_mesh.add_socket(s)
    
    s.set_editor_property("bone_name", "hand_r")
    s.set_editor_property("relative_location", unreal.Vector(-10.764, 5.811, -0.728))
    # Rotation matching standard weapon_r socket in UE4/UE5 mannequins
    s.set_editor_property("relative_rotation", unreal.Rotator(pitch=0.0, yaw=-180.0, roll=0.0))
    eal.save_loaded_asset(robot_mesh)
    print("Saved weapon_r socket on SK_Robot_scout_R21")

# Also add weapon_r socket to skeleton if accessible
skel = robot_mesh.get_editor_property("skeleton")
if skel:
    s_skel = skel.find_socket("weapon_r")
    if not s_skel:
        s_skel = unreal.SkeletalMeshSocket(outer=skel)
        s_skel.set_editor_property("socket_name", "weapon_r")
        skel.add_socket(s_skel)
    s_skel.set_editor_property("bone_name", "hand_r")
    s_skel.set_editor_property("relative_location", unreal.Vector(-10.764, 5.811, -0.728))
    s_skel.set_editor_property("relative_rotation", unreal.Rotator(pitch=0.0, yaw=-180.0, roll=0.0))
    eal.save_loaded_asset(skel)
    print("Saved weapon_r socket on Skeleton")

# 2. Add Grip socket on SM_LongSword
longsword = unreal.load_asset("/Game/RR/Weapons/Fab/long_sword/StaticMeshes/SM_LongSword")
if longsword:
    g = longsword.find_socket("Grip")
    if not g:
        g = unreal.StaticMeshSocket(outer=longsword)
        g.set_editor_property("socket_name", "Grip")
        longsword.add_socket(g)
    
    # Handle center X = -56.735. 
    # Blade is along +X, edge is along +Y, flat is along +Z.
    # In weapon_r: Up is +Z, Forward is +X, Right is +Y.
    # To rotate mesh so blade (+X) -> +Z (out of fist): Pitch = -90.
    # InvGrip will apply: Inv(Rot(pitch=90)) -> Rot(pitch=-90) on the mesh!
    # Let's test Grip relative rotation:
    # If Grip has relative_location = (-56.735, 0, 0) and relative_rotation = (pitch=90.0, yaw=0.0, roll=0.0):
    # GripTransform = Loc(-56.735, 0, 0), Rot(90, 0, 0)
    # InvGrip = GripTransform.Inverse() -> Loc(+56.735 rotated), Rot(-90, 0, 0)!
    # Then mesh rotation in weapon actor will be Rot(-90, 0, 0), which rotates +X (blade) to +Z!
    g.set_editor_property("relative_location", unreal.Vector(-56.735, 0.0, 0.0))
    g.set_editor_property("relative_rotation", unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0))
    
    # TraceStart and TraceEnd for combat hitbox
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
    print("Saved Grip, TraceStart, TraceEnd sockets on SM_LongSword")

# 3. Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    vis = da.get_editor_property("visuals")
    vis.set_editor_property("attachment_socket", "weapon_r")
    vis.set_editor_property("hand_offset", unreal.Transform(unreal.Vector(0,0,0), unreal.Rotator(0,0,0), unreal.Vector(1,1,1)))
    da.set_editor_property("visuals", vis)
    eal.save_loaded_asset(da)
    print("Saved DA_PlayerWeapon with attachment_socket=weapon_r, hand_offset=Identity")

# 4. Configure L_RRArena level: PlayerStart, Arena Center, EntryPoint, Fighting Range
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
    
    # Trace interior flat green circular ground
    def trace_ground(x, y, z_start=6000.0, z_end=-6000.0):
        hit = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(x, y, z_start), unreal.Vector(x, y, z_end),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
        )
        if hit:
            t = hit.to_tuple()
            return t[5].z, t[9]
        return None, None

    floor_z, f_actor = trace_ground(CX, CY)
    if floor_z is None:
        floor_z = CZ
    print(f"Arena floor at Z={floor_z}")

    # The green circular arena has radius ~1750.
    # Mid-part green area is inside Radius = 1750.
    # The user says:
    # "seocnd thign i must spawn in the outside of the arena then in the filed effect when i enter then only should i be able to enter in the arena"
    # "also know that in areana the fghtinin range is only mid part green not the entire since i am spannign in the staris at the end almost"
    
    # Outside the arena stairs:
    # Stairs lead up towards outside (e.g. at (-1800, -1000) or similar along the entrance corridor)
    # Let's inspect existing PlayerStart position
    ps_actors = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
    
    # Find ARRArena actor
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

    # Duel spots inside the green ring
    def set_rel(c_name, rx, ry, rz):
        try:
            c = arena.get_editor_property(c_name)
            c.set_editor_property("relative_location", unreal.Vector(rx, ry, rz))
        except Exception as ex:
            print(f"Failed to set {c_name}: {ex}")

    set_rel("player_spot", 0.0, -600.0, 50.0)
    set_rel("boss_spot", 0.0, 600.0, 50.0)
    set_rel("boss_entrance", 0.0, 1100.0, 50.0)

    # Entry point: at the boundary of the green floor entering from the stairs
    # The entrance staircase enters from around X = CX - 1650, Y = CY - 850 (or along the major entrance)
    # Let's check where the stairs/entrance is:
    # In earlier setup_colosseum_arena.py, entry_wx = 2400.0, entry_wy = 400.0 was on outside ground
    # Let's check where PlayerStart currently is
    if ps_actors:
        ps = ps_actors[0]
        cur_ps_loc = ps.get_actor_location()
        print(f"Current PlayerStart loc: ({cur_ps_loc.x:.1f}, {cur_ps_loc.y:.1f}, {cur_ps_loc.z:.1f})")

    # Set EntryPoint field effect on the stairs/archway entrance leading into the green arena:
    # Distance from arena center ~1850 (just outside the 1750 boundary so the player enters the field effect to trigger the duel!)
    # Direction towards PlayerStart:
    if ps_actors:
        ps_loc = ps_actors[0].get_actor_location()
        dir_to_ps = (ps_loc - unreal.Vector(CX, CY, floor_z)).get_safe_normal()
        entry_loc = unreal.Vector(CX, CY, floor_z) + dir_to_ps * 1800.0
        entry_z, _ = trace_ground(entry_loc.x, entry_loc.y)
        if entry_z is not None:
            entry_loc.z = entry_z + 20.0
        
        try:
            ep = arena.get_editor_property("entry_point")
            ep.set_world_location(entry_loc)
            print(f"EntryPoint placed at ({entry_loc.x:.1f}, {entry_loc.y:.1f}, {entry_loc.z:.1f})")
            xp = arena.get_editor_property("exit_point")
            xp.set_world_location(entry_loc + dir_to_ps * 200.0)
        except Exception as ex:
            print(f"Failed to set entry point: {ex}")

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    print("SUCCESS: L_RRArena saved with configured Arena and EntryPoint!")

print("SETUP_SOCKETS_AND_VERIFY_COMPLETE")
