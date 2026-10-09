import unreal

# Test if we can import t3d
task = unreal.AssetImportTask()
task.filename = unreal.Paths.project_saved_dir() + "RRTools/ABP_GenericRetarget.t3d"
task.destination_path = "/Game/Blueprints/RetargetedCharacters"
task.destination_name = "ABP_GenericRetarget_Test"
task.replace_existing = True
task.automated = True
task.save = False

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
test_asset = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget_Test")
print("TEST_IMPORT_RESULT:", test_asset)
