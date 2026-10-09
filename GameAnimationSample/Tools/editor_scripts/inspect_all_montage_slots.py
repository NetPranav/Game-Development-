import unreal, json

ar = unreal.AssetRegistryHelpers.get_asset_registry()
montages = ar.get_assets(unreal.ARFilter(
    package_paths=["/Game/RR/Greystone", "/Game/ParagonGreystone"],
    recursive_paths=True,
    class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage")]
))

out = {}
for a in montages:
    m = a.get_asset()
    if m:
        slots = []
        for track in m.get_editor_property("slot_anim_tracks"):
            slots.append(str(track.get_editor_property("slot_name")))
        out[str(a.asset_name)] = {
            "path": str(a.package_name),
            "slots": slots
        }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_all_greystone_montages.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ALL_MONTAGE_SLOTS_DONE")
