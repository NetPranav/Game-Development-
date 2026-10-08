import unreal, json
ar = unreal.AssetRegistryHelpers.get_asset_registry()
eal = unreal.EditorAssetLibrary
log = {"changed": [], "skipped": 0}
for root, cap in (("/Game/Robot_scout_R_21", 1024), ("/Game/Stylized_Dark_Sword", 1024)):
    flt = unreal.ARFilter(package_paths=[root], recursive_paths=True, class_names=["Texture2D"])
    for a in ar.get_assets(flt):
        t = a.get_asset()
        try:
            w = t.blueprint_get_size_x(); h = t.blueprint_get_size_y()
            cur = t.get_editor_property("max_texture_size")
            if max(w, h) > cap and (cur == 0 or cur > cap):
                t.set_editor_property("max_texture_size", cap)
                eal.save_loaded_asset(t)
                log["changed"].append("%s %dx%d -> %d" % (a.asset_name, w, h, cap))
            else:
                log["skipped"] += 1
        except Exception as e:
            log["err_" + str(a.asset_name)] = str(e)[:80]
open(unreal.Paths.project_saved_dir() + "RRTools/shrink_out.json", "w").write(json.dumps(log, indent=1))
