import unreal, json

unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

out = {}

col_actors = [a for a in acts if "Colosseum" in a.get_actor_label()]
out["colosseum_actors"] = []
for c in col_actors:
    o, e = c.get_actor_bounds(False)
    loc = c.get_actor_location()
    scale = c.get_actor_scale3d()
    out["colosseum_actors"].append({
        "label": c.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z],
        "scale": [scale.x, scale.y, scale.z],
        "bounds_origin": [o.x, o.y, o.z],
        "bounds_extent": [e.x, e.y, e.z]
    })

arenas = [a for a in acts if a.get_class().get_name() == "RRArena"]
out["arenas"] = []
for a in arenas:
    loc = a.get_actor_location()
    out["arenas"].append({
        "label": a.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z],
        "radius": a.get_editor_property("radius")
    })

ps_list = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
out["player_starts"] = []
for p in ps_list:
    loc = p.get_actor_location()
    out["player_starts"].append({
        "label": p.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z]
    })

# Trace downward at arena center
hit = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(3750.0, -910.0, 5000.0), unreal.Vector(3750.0, -910.0, -5000.0),
    unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
)
if hit:
    t = hit.to_tuple()
    out["floor_trace_at_center"] = {
        "hit_actor": t[9].get_actor_label() if t[9] else None,
        "impact_z": t[5].z
    }
else:
    out["floor_trace_at_center"] = None

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_arena_colosseum.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ARENA_COLOSSEUM_DONE")
