import unreal, json

ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/IKRig", "IKRetargeter"))

out = []
for a in assets:
    out.append({
        "package_name": str(a.package_name),
        "asset_name": str(a.asset_name)
    })

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_all_rtgs.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ALL_RTGS_DONE")
