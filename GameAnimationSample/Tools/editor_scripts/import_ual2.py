"""Imports the Quaternius Universal Animation Library 2 (CC0) GLB into /Game/RR/ExternalAnims/UAL2."""
import unreal

SRC = r"A:\UNREAL PROJECT\Game\GameAnimationSample\ExternalAnims\UAL2\ex\Universal Animation Library 2 [Standard]\Unreal-Godot\UAL2_Standard.glb"
DEST = "/Game/RR/ExternalAnims/UAL2"

t = unreal.AssetImportTask()
t.filename = SRC
t.destination_path = DEST
t.replace_existing = True
t.automated = True
t.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
print("imported objects:", [str(x) for x in t.get_editor_property("imported_object_paths")][:10])
assets = unreal.EditorAssetLibrary.list_assets(DEST, recursive=True)
print("count", len(assets))
for a in assets[:80]:
    print("ASSET", a)
