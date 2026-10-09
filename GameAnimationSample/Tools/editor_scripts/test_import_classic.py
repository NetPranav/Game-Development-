import unreal

unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
print("Interchange FBX disabled, testing single anim import...")

WS = "A:/UNREAL PROJECT/Game/GameAnimationSample"
anim_src = f"{WS}/ExternalAnims/ProSwordShield/sword and shield slash.fbx"
anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"

skeleton = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base_Skeleton")

t = unreal.AssetImportTask()
t.filename = anim_src
t.destination_path = anim_dst_dir
t.destination_name = "test_slash"
t.automated = True
t.save = True
t.replace_existing = True

ui = unreal.FbxImportUI()
ui.import_mesh = False
ui.import_animations = True
ui.skeleton = skeleton
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
t.options = ui

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
print("Imported:", t.imported_object_paths)
