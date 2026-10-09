import unreal, math, json

print("=== SETTING UP COLOSSEUM AND ARENA IN L_RRArena ===")

LEVEL = "/Game/RR/Levels/L_RRArena"
unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

# 1. Find Colosseum Actor
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
    # Check StaticMeshActor_47 specifically
    for a in acts:
        if a.get_actor_label() == "StaticMeshActor_47":
            col_actor = a
            break

if not col_actor:
    print("ERROR: Colosseum actor not found!")
else:
    c_loc = col_actor.get_actor_location()
    c_rot = col_actor.get_actor_rotation()
    CX, CY, CZ = c_loc.x, c_loc.y, c_loc.z
    yaw = c_rot.yaw
    print(f"Found Colosseum at ({CX:.1f}, {CY:.1f}, {CZ:.1f}) with Yaw={yaw:.1f}")

    def trace_ground(x, y, z_start=6000.0, z_end=-6000.0):
        hit = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(x, y, z_start), unreal.Vector(x, y, z_end),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
        )
        if hit:
            t = hit.to_tuple()
            return t[5].z, t[9]
        return None, None

    # Find interior floor Z at center
    floor_z, f_actor = trace_ground(CX, CY)
    print(f"Interior floor at ({CX}, {CY}) is Z={floor_z} (hit: {f_actor.get_actor_label() if f_actor else 'None'})")
    if floor_z is None:
        floor_z = CZ + 50.0

    # Yaw in radians
    rad_yaw = math.radians(yaw)
    # Local X forward direction (perpendicular to major axis, pointing towards entrance / rest of map)
    # Let's check which side (-X or +X) is towards the main landscape
    fx_neg = -math.cos(rad_yaw)
    fy_neg = -math.sin(rad_yaw)
    
    # Exterior main ground points (safe, flat starter terrain outside the Colosseum):
    entry_wx = 2400.0
    entry_wy = 400.0
    entry_wz, e_actor = trace_ground(entry_wx, entry_wy)
    if entry_wz is None:
        entry_wz = 50.0
    print(f"Entrance ground at ({entry_wx:.1f}, {entry_wy:.1f}) is Z={entry_wz:.1f} (hit: {e_actor.get_actor_label() if e_actor else 'None'})")

    # 2. Update / Setup ARRArena at Colosseum center
    arenas = [a for a in acts if a.get_class().get_name() == "RRArena"]
    if not arenas:
        cls = unreal.load_class(None, "/Script/RRGame.RRArena")
        arena = eas.spawn_actor_from_class(cls, unreal.Vector(CX, CY, floor_z), unreal.Rotator(0, yaw, 0))
        arena.set_actor_label("RR_DuelArena")
    else:
        arena = arenas[0]
        arena.set_actor_location(unreal.Vector(CX, CY, floor_z), False, True)
        arena.set_actor_rotation(unreal.Rotator(0, yaw, 0), False)

    arena.set_editor_property("radius", 1250.0)
    arena.set_editor_property("entry_radius", 220.0)
    arena.set_editor_property("entry_hold_seconds", 0.5)

    # Duel line on flat interior circular floor
    def set_rel(c_name, rx, ry, rz):
        try:
            c = arena.get_editor_property(c_name)
            c.set_editor_property("relative_location", unreal.Vector(rx, ry, rz))
        except Exception as ex:
            print(f"Failed to set {c_name}: {ex}")

    set_rel("player_spot", 0.0, -550.0, 50.0)
    set_rel("boss_spot", 0.0, 550.0, 50.0)
    set_rel("boss_entrance", 0.0, 950.0, 50.0)
    
    # Entry and Exit points on main ground outside
    try:
        ep = arena.get_editor_property("entry_point")
        ep.set_world_location(unreal.Vector(entry_wx, entry_wy, entry_wz + 5.0))
        xp = arena.get_editor_property("exit_point")
        xp.set_world_location(unreal.Vector(2150.0, 450.0, entry_wz + 50.0))
    except Exception as ex:
        print(f"Failed to set entry/exit world loc: {ex}")

    # 3. Update PlayerStart on main flat ground outside, facing the entrance circle & Colosseum
    ps_wx = 2000.0
    ps_wy = 400.0
    ps_wz, ps_actor = trace_ground(ps_wx, ps_wy)
    if ps_wz is None:
        ps_wz = entry_wz

    ps_list = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
    if ps_list:
        ps = ps_list[0]
        ps.set_actor_location(unreal.Vector(ps_wx, ps_wy, ps_wz + 92.0), False, True)
        face_yaw = math.degrees(math.atan2(CY - ps_wy, CX - ps_wx))
        ps.set_actor_rotation(unreal.Rotator(0, face_yaw, 0), False)
        print(f"PlayerStart placed on main ground at ({ps_wx:.1f}, {ps_wy:.1f}, {ps_wz + 92.0:.1f}) with Yaw={face_yaw:.1f} (ground: {ps_actor.get_actor_label() if ps_actor else 'None'})")

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    print("SUCCESS: Arena and PlayerStart setup complete and saved to level!")

out = {
    "col_center": [CX, CY, CZ],
    "floor_z": floor_z,
    "entry_pos": [entry_wx, entry_wy, entry_wz],
    "ps_pos": [ps_wx, ps_wy, ps_wz + 92.0]
}
with open(unreal.Paths.project_saved_dir() + "RRTools/setup_arena_out.json", "w") as f:
    json.dump(out, f, indent=2)

print("SETUP_COLOSSEUM_ARENA_COMPLETE")
