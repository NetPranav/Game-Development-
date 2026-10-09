import unreal, json, math

world = unreal.EditorLevelLibrary.get_editor_world() if hasattr(unreal, "EditorLevelLibrary") else None
# Or load level L_RRArena
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
acts = eas.get_all_level_actors()

col = None
for a in acts:
    if "colosseum" in a.get_actor_label().lower():
        col = a
        break

if col:
    loc = col.get_actor_location()
    cx, cy, cz = loc.x, loc.y, loc.z
    scale = col.get_actor_scale3d()
    print(f"Colosseum at ({cx}, {cy}, {cz}), scale: ({scale.x}, {scale.y}, {scale.z})")
    
    # Trace along +X direction at intervals
    profile = []
    w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    for r in range(0, 4200, 100):
        # Trace down at (cx + r, cy)
        px = cx + r
        py = cy
        hit = unreal.SystemLibrary.line_trace_single(
            w, unreal.Vector(px, py, cz + 3000.0), unreal.Vector(px, py, cz - 1000.0),
            unreal.TraceTypeQuery.ECC_VISIBILITY, False, [], unreal.DrawDebugTrace.NONE, True
        )
        if hit:
            t = hit.to_tuple()
            profile.append({
                "radius": r,
                "hit_actor": t[9].get_actor_label() if t[9] else "None",
                "impact_z": t[5].z,
                "rel_z": t[5].z - cz
            })
        else:
            profile.append({"radius": r, "hit": False})
            
    with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/col_profile.json", "w") as f:
        json.dump({"center": [cx, cy, cz], "scale": [scale.x, scale.y, scale.z], "profile": profile}, f, indent=2)

print("COL_PROFILE_DONE")
