import unreal, json

eal = unreal.EditorAssetLibrary
mesh = unreal.load_asset("/Game/RR/Environment/Colosseum/SM_Colosseum")

out = {}
if mesh:
    nanite_settings = mesh.get_editor_property("nanite_settings")
    out["nanite_enabled"] = nanite_settings.get_editor_property("enabled")
    
    body_setup = mesh.get_editor_property("body_setup")
    if body_setup:
        out["collision_complexity"] = str(body_setup.get_editor_property("collision_trace_flag"))
    
    bounds = mesh.get_bounds()
    out["bounds_origin"] = [bounds.origin.x, bounds.origin.y, bounds.origin.z]
    out["bounds_extent"] = [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z]
    
with open(unreal.Paths.project_saved_dir() + "RRTools/colosseum_info.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_COLOSSEUM_DONE")
