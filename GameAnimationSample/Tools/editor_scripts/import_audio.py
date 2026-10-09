import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
SRC = "A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalAudio/oga/"
files = {"hot_spring_town_1.ogg": "MUS_Explore_HotSprings", "qubodup-yd-DarkShrineLoop-OpenGameArt.ogg": "MUS_Shrine_Tension",
         "determined_pursuit_loop.wav": "MUS_Battle_DeterminedPursuit", "wind1.wav": "AMB_Wind"}
tasks = []
for f, name in files.items():
    t = unreal.AssetImportTask()
    t.filename = SRC + f; t.destination_path = "/Game/RR/Audio"; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for t in tasks:
    for p in t.imported_object_paths:
        a = unreal.load_asset(p)
        if isinstance(a, unreal.SoundWave):
            a.set_editor_property("looping", True)
            unreal.EditorAssetLibrary.save_loaded_asset(a)
        print("AUD", p, type(a).__name__, a.get_editor_property("duration") if isinstance(a, unreal.SoundWave) else "")
# list existing sound assets that could serve as combat SFX
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for cls in ("SoundCue", "SoundWave", "MetaSoundSource"):
    pk = "/Script/Engine" if cls != "MetaSoundSource" else "/Script/MetasoundEngine"
    for a in ar.get_assets(unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath(pk, cls)], package_paths=["/Game"], recursive_paths=True)):
        n = str(a.asset_name).lower()
        if any(k in n for k in ("impact", "sword", "hit", "clash", "metal", "swing", "whoosh", "block", "parry", "crowd", "cheer", "slash", "footstep", "body")):
            print("SFX", cls, a.package_name)
