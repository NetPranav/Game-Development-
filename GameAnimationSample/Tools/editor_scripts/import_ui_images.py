"""Imports the menu / loading-screen key art from <project>/Images into /Game/RR/UI with UI-friendly texture settings.

Run headless:
  UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=Tools/editor_scripts/import_ui_images.py -unattended -nosplash -nullrhi

Drop a square PNG named T_KaizeenLogo into /Game/RR/UI (or re-run with LOGO below) and the splash screen uses it
instead of the procedural emblem.
"""
import os
import unreal

SRC = os.path.join(unreal.Paths.project_dir(), "Images")
DEST = "/Game/RR/UI"

# (file, asset name). The duplicated "a84c7fe6 (1).jpg" is intentionally skipped.
ITEMS = [
    ("Starting.png", "T_RR_Start"),
    ("4d2d11a9-f9ee-4894-ba43-6538f33c2cd4.jpg", "T_RR_Load_1"),
    ("79530df2-beec-42c0-8323-acc51534b7dc.jpg", "T_RR_Load_2"),
    ("a1599eed-8dea-46b0-b60e-193c1bc06bcd.jpg", "T_RR_Load_3"),
    ("a84c7fe6-11ee-4809-a201-3ae7146e31cd.jpg", "T_RR_Load_4"),
]
LOGO = ("Kaizeen_logo.png", "T_KaizeenLogo")  # optional

tasks = []
items = list(ITEMS)
if os.path.exists(os.path.join(SRC, LOGO[0])):
    items.append(LOGO)

for fname, asset in items:
    path = os.path.join(SRC, fname)
    if not os.path.exists(path):
        unreal.log_warning("missing image: " + path)
        continue
    t = unreal.AssetImportTask()
    t.filename = path
    t.destination_path = DEST
    t.destination_name = asset
    t.replace_existing = True
    t.automated = True
    t.save = False
    tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for fname, asset in items:
    ap = "%s/%s" % (DEST, asset)
    tex = unreal.load_asset(ap)
    if not tex:
        unreal.log_warning("not imported: " + ap)
        continue
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property("never_stream", True)
    tex.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(tex)
    unreal.log("imported " + ap + " %dx%d" % (tex.blueprint_get_size_x(), tex.blueprint_get_size_y()))
