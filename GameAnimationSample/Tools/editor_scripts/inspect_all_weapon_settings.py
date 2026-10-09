import unreal, json

ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets(unreal.ARFilter(
    recursive_paths=True, 
    package_paths=["/Game"], 
    class_paths=[unreal.TopLevelAssetPath("/Script/AAACombat", "AAAWeaponSettings")]
))

out = {}
for a in assets:
    da = a.get_asset()
    v = da.get_editor_property("visuals")
    ho = v.get_editor_property("hand_offset")
    mesh = v.get_editor_property("static_mesh")
    out[da.get_name()] = {
        "attachment_socket": str(v.get_editor_property("attachment_socket")),
        "mesh": str(mesh.get_path_name()) if mesh else None,
        "hand_offset": {
            "loc": [ho.translation.x, ho.translation.y, ho.translation.z],
            "rot": [ho.rotation.rotator().pitch, ho.rotation.rotator().yaw, ho.rotation.rotator().roll]
        }
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_all_weapon_settings.json", "w") as f:
    json.dump(out, f, indent=2)

print("ALL_WEAPON_SETTINGS_DONE")
