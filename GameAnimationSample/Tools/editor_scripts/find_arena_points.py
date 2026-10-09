import unreal, json

unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

out = {}

# 1. Colosseum info
col_actors = [a for a in acts if a.get_actor_label() == "RR_Colosseum" or "Colosseum" in a.get_actor_label()]
out["col_actors"] = []
for c in col_actors:
    loc = c.get_actor_location()
    scale = c.get_actor_scale3d()
    o, e = c.get_actor_bounds(False)
    out["col_actors"].append({
        "label": c.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z],
        "scale": [scale.x, scale.y, scale.z],
        "origin": [o.x, o.y, o.z],
        "extent": [e.x, e.y, e.z]
    })

# Helper trace
def trace_down(x, y, z_start=10000.0, z_end=-5000.0):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, z_start), unreal.Vector(x, y, z_end),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
    )
    if hit:
        t = hit.to_tuple()
        return {
            "hit_actor": t[9].get_actor_label() if t[9] else "None",
            "impact_x": t[5].x,
            "impact_y": t[5].y,
            "impact_z": t[5].z
        }
    return None

# Test traces
traces = {}
# Center of colosseum
c_loc = col_actors[0].get_actor_location() if col_actors else unreal.Vector(3750, -910, 0)
traces["center"] = trace_down(c_loc.x, c_loc.y)
traces["center_offset_x"] = trace_down(c_loc.x + 300, c_loc.y)
traces["center_offset_y"] = trace_down(c_loc.x, c_loc.y + 300)

# Current PlayerStart
ps_list = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
if ps_list:
    ps_loc = ps_list[0].get_actor_location()
    traces["at_player_start"] = trace_down(ps_loc.x, ps_loc.y)

# Current Arena actor
arenas = [a for a in acts if a.get_class().get_name() == "RRArena"]
if arenas:
    ar = arenas[0]
    ar_loc = ar.get_actor_location()
    traces["at_arena_actor"] = trace_down(ar_loc.x, ar_loc.y)
    
    # Check arena component locations
    comps = {}
    for c_name in ["player_spot", "boss_spot", "entry_point", "exit_point", "boss_entrance"]:
        try:
            c = ar.get_editor_property(c_name)
            wloc = c.get_world_location()
            comps[c_name] = [wloc.x, wloc.y, wloc.z]
        except Exception as e:
            comps[c_name] = str(e)
    out["arena_components"] = comps

out["traces"] = traces

with open(unreal.Paths.project_saved_dir() + "RRTools/arena_traces.json", "w") as f:
    json.dump(out, f, indent=2)

print("ARENA_TRACES_DONE")
