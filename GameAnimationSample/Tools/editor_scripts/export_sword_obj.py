import unreal, json

task = unreal.AssetExportTask()
task.object = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
task.filename = unreal.Paths.project_saved_dir() + "RRTools/SM_Sword_A.obj"
task.exporter = unreal.StaticMeshExporterOBJ()
task.automated = True
task.prompt = False

res = unreal.Exporter.run_asset_export_task(task)
print(f"EXPORT_RES: {res}")
