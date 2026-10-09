import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
for a in eas.get_all_level_actors():
    cn = a.get_class().get_name(); lb = a.get_actor_label()
    if any(k in (cn + lb) for k in ("Colosseum", "PlayerStart", "RRArena", "Cylinder", "Landscape", "NavMesh", "Stair", "Step")) or "Plane" == lb:
        o, e = a.get_actor_bounds(False)
        print("COL %-22s %-30s loc %s rot %s scale %s bounds c=(%.0f,%.0f,%.0f) e=(%.0f,%.0f,%.0f)" % (cn[:22], lb[:30], a.get_actor_location(), a.get_actor_rotation(), a.get_actor_scale3d(), o.x, o.y, o.z, e.x, e.y, e.z))
def trace(x, y, z0, z1):
    h = unreal.SystemLibrary.line_trace_single(world, unreal.Vector(x, y, z0), unreal.Vector(x, y, z1), unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True)
    if not h: return None
    t = h.to_tuple(); return (round(t[5].z), t[9].get_actor_label() if t[9] else None)
col = [a for a in eas.get_all_level_actors() if "Colosseum" in a.get_actor_label() or "Colosseum" in a.get_name()]
if col:
    c = col[0].get_actor_location()
    print("COL center", c)
    for dz in (3500, 1500, 600, 300):
        print("COL trace from +%d" % dz, trace(c.x, c.y, c.z + dz, c.z - 6000))
    # radial floor profile from centre (first hit from 400 above the centre floor)
    fz = trace(c.x, c.y, c.z + 300, c.z - 3000)
    for ang in (0, 90, 180, 270, 208):
        import math
        row = []
        for r in range(0, 4001, 250):
            x = c.x + math.cos(math.radians(ang)) * r; y = c.y + math.sin(math.radians(ang)) * r
            h = trace(x, y, c.z + 2500, c.z - 3000)
            row.append("%d:%s" % (r, h[0] if h else "-"))
        print("COL ray %3d" % ang, " ".join(row))
