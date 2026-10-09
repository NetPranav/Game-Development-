import unreal

task = unreal.AssetExportTask()
task.object = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
task.filename = unreal.Paths.project_saved_dir() + "RRTools/ABP_GenericRetarget.t3d"
task.selected = False
task.replace_identical = True
task.prompt = False
task.automated = True

exporter = unreal.BlueprintFactory() # or use Exporter.run_asset_export_task
result = unreal.Exporter.run_asset_export_task(task)
print("EXPORT_RESULT:", result)
