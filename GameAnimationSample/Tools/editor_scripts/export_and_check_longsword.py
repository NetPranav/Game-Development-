import unreal, json

eal = unreal.EditorAssetLibrary

mesh = unreal.load_asset("/Game/RR/Weapons/Fab/long_sword/StaticMeshes/SM_LongSword")
out = {}
if mesh:
    b = mesh.get_bounds()
    out["bounds"] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z]
    }
    
    # Export OBJ
    task = unreal.AssetExportTask()
    task.object = mesh
    task.filename = unreal.Paths.project_saved_dir() + "RRTools/SM_LongSword.obj"
    task.exporter = unreal.StaticMeshExporterOBJ()
    task.automated = True
    task.prompt = False
    res = unreal.Exporter.run_asset_export_task(task)
    out["exported"] = res

with open(unreal.Paths.project_saved_dir() + "RRTools/export_longsword.json", "w") as f:
    json.dump(out, f, indent=2)

print("EXPORT_LONGSWORD_COMPLETE")
