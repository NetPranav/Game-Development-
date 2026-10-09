import unreal, math
# Places ARRArena on the round stone platform of L_RRArena and moves the PlayerStart to the ground beside the entry circle.
# Platform ("Cylinder") centre ~ (3750, -910), floor radius ~1136, post ring ~1000, gap in the posts toward (3966, 60).
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
LEVEL = "/Game/RR/Levels/L_RRArena"
unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

CX, CY = 3750.0, -910.0
cyl = [a for a in acts if a.get_actor_label() == "Cylinder"]
if cyl:
    o, e = cyl[0].get_actor_bounds(False)
    CX, CY = o.x, o.y
    print("ARENA platform bounds", o, e)

def ground(x, y, z0=3000.0, z1=-3000.0, ignore=None):
    hit = unreal.SystemLibrary.line_trace_single(world, unreal.Vector(x, y, z0), unreal.Vector(x, y, z1),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, ignore or [], unreal.DrawDebugTrace.NONE, True)
    if hit:
        t = hit.to_tuple()
        # (blocking_hit, initial_overlap, time, distance, location, impact_point, normal, impact_normal, phys_mat, hit_actor, ...)
        return t[5].z, t[9]
    return None, None

top, top_actor = ground(CX, CY)
print("ARENA floor z", top, top_actor.get_actor_label() if top_actor else None)

old = [a for a in acts if a.get_class().get_name() == "RRArena"]
for a in old:
    eas.destroy_actor(a)
cls = unreal.load_class(None, "/Script/RRGame.RRArena")
arena = eas.spawn_actor_from_class(cls, unreal.Vector(CX, CY, (top or 180.0)), unreal.Rotator(0, 0, 0))
arena.set_actor_label("RR_DuelArena")
arena.set_editor_property("radius", 960.0)

ps = [a for a in acts if a.get_class().get_name() == "PlayerStart"][0]
p0 = ps.get_actor_location()
dx, dy = p0.x - CX, p0.y - CY
L = math.hypot(dx, dy) or 1.0
ex, ey = dx / L, dy / L                 # platform -> current spawn direction (the entry side)
px, py = ey, -ex                        # perpendicular
gx, gy = (3966.0 - CX), (60.0 - CY)
gl = math.hypot(gx, gy); gx, gy = gx / gl, gy / gl   # toward the gap in the posts

def rel(comp_name, x, y, z):
    c = arena.get_editor_property(comp_name)
    c.set_editor_property("relative_location", unreal.Vector(x, y, z))

rel("player_spot", ex * 420, ey * 420, 100)
rel("boss_spot", -ex * 420, -ey * 420, 100)
rel("entry_point", ex * 1420, ey * 1420, -100)
rel("exit_point", ex * 1680 + px * 260, ey * 1680 + py * 260, 0)
rel("boss_entrance", gx * 1060, gy * 1060, 100)

# PlayerStart: on the ground just beyond the entry circle, facing the platform
sx, sy = CX + ex * 1850, CY + ey * 1850
gz, gactor = ground(sx, sy)
if gz is not None:
    ps.set_actor_location(unreal.Vector(sx, sy, gz + 100.0), False, True)
    yaw = math.degrees(math.atan2(-ey, -ex))
    ps.set_actor_rotation(unreal.Rotator(0, 0, yaw), False)
    print("ARENA playerstart moved to", sx, sy, gz + 100.0, "yaw", yaw, "ground", gactor.get_actor_label() if gactor else None)
ez, eactor = ground(CX + ex * 1420, CY + ey * 1420)
print("ARENA entry ground z", ez, eactor.get_actor_label() if eactor else None)

unreal.EditorLoadingAndSavingUtils.save_current_level()
print("ARENA_DONE", arena.get_actor_location())
