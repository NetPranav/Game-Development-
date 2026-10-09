import unreal, os
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
B = "A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalAudio/sword/"
jobs = []
for i in range(1, 11):
    jobs.append((B + "x/atk/sword - StarNinjas/sword.%d.ogg" % i, "SFX_Sword_%02d" % i))
    jobs.append((B + "x/clash/sword_clash.%d.ogg" % i, "SFX_Clash_%02d" % i))
jobs.append((B + "crowd_shouting_0.ogg", "SFX_CrowdShout"))
tasks = []
for f, n in jobs:
    t = unreal.AssetImportTask(); t.filename = f; t.destination_path = "/Game/RR/Audio/SFX"; t.destination_name = n
    t.automated = True; t.replace_existing = True; t.save = True; tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
print("SFXI", sum(len(t.imported_object_paths) for t in tasks), "imported")
