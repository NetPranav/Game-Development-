import unreal, json

print("=== CHECKING ARENA SETUP ===")
LEVEL = "/Game/RR/Levels/L_RRArena"
unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
acts = eas.get_all_level_actors()

data = {}

# 1. Colosseum
cols = [a for a in acts if "Colosseum" in a.get_actor_label() or "colosseum" in a.get_actor_label().lower()]
data["colosseum"] = []
for c in cols:
    loc = c.get_actor_location()
    rot = c.get_actor_rotation()
    scale = c.get_actor_scale3d()
    o, e = c.get_actor_bounds(False)
    data["colosseum"].append({
        "label": c.get_actor_label(),
        "class": c.get_class().get_name(),
        "loc": [loc.x, loc.y, loc.z],
        "rot": [rot.pitch, rot.yaw, rot.roll],
        "scale": [scale.x, scale.y, scale.z],
        "bounds_origin": [o.x, o.y, o.z],
        "bounds_extent": [e.x, e.y, e.z]
    })

# 2. Arena
arenas = [a for a in acts if a.get_class().get_name() == "RRArena"]
data["arena"] = []
for a in arenas:
    loc = a.get_actor_location()
    comps = {}
    for c_name in ["PlayerSpot", "BossSpot", "EntryPoint", "ExitPoint", "BossEntrance", "EntryDisc", "EntryGlow"]:
        try:
            c = a.get_editor_property(c_name.lower()) if hasattr(a, c_name.lower()) else a.get_editor_property(c_name)
            wloc = c.get_world_location()
            rloc = c.get_relative_location()
            comps[c_name] = {
                "world": [wloc.x, wloc.y, wloc.z],
                "rel": [rloc.x, rloc.y, rloc.z]
            }
        except Exception as ex:
            comps[c_name] = str(ex)
    data["arena"].append({
        "label": a.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z],
        "radius": a.get_editor_property("radius"),
        "comps": comps
    })

# 3. PlayerStart
pss = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
data["player_starts"] = []
for p in pss:
    loc = p.get_actor_location()
    rot = p.get_actor_rotation()
    data["player_starts"].append({
        "label": p.get_actor_label(),
        "loc": [loc.x, loc.y, loc.z],
        "rot": [rot.pitch, rot.yaw, rot.roll]
    })

# 4. Traces
def trace(x, y, z_start=6000.0, z_end=-6000.0):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, z_start), unreal.Vector(x, y, z_end),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True
    )
    if hit:
        t = hit.to_tuple()
        return {
            "hit_actor": t[9].get_actor_label() if t[9] else "None",
            "impact_z": t[5].z
        }
    return None

traces = {}
if arenas and "arena" in data and data["arena"]:
    ar_loc = data["arena"][0]["loc"]
    traces["arena_center"] = trace(ar_loc[0], ar_loc[1])
    comps = data["arena"][0]["comps"]
    for k in ["PlayerSpot", "BossSpot", "EntryPoint", "ExitPoint", "BossEntrance"]:
        if k in comps and isinstance(comps[k], dict):
            wl = comps[k]["world"]
            traces[k] = trace(wl[0], wl[1])

if pss:
    p_loc = pss[0].get_actor_location()
    traces["player_start"] = trace(p_loc.x, p_loc.y)

data["traces"] = traces

out_path = unreal.Paths.project_saved_dir() + "RRTools/arena_status.json"
with open(out_path, "w") as f:
    json.dump(data, f, indent=2)

print(f"ARENA_STATUS_SAVED to {out_path}")
