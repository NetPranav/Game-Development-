import unreal, json

ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/AAACombat", "AAAWeaponSettings"))

out = {}
for a in assets:
    da = a.get_asset()
    if not da:
        continue
    vis = da.get_editor_property("visuals")
    ho = vis.get_editor_property("hand_offset")
    holo = vis.get_editor_property("holster_offset")
    sm = vis.get_editor_property("static_mesh")
    skm = vis.get_editor_property("skeletal_mesh")
    
    out[str(a.package_name)] = {
        "attachment_socket": str(vis.get_editor_property("attachment_socket")),
        "holster_socket": str(vis.get_editor_property("holster_socket")),
        "static_mesh": str(sm.get_path_name()) if sm else None,
        "skeletal_mesh": str(skm.get_path_name()) if skm else None,
        "hand_offset": {
            "loc": [ho.translation.x, ho.translation.y, ho.translation.z],
            "rot": [ho.rotation.rotator().pitch, ho.rotation.rotator().yaw, ho.rotation.rotator().roll],
            "scale": [ho.scale3d.x, ho.scale3d.y, ho.scale3d.z]
        },
        "holster_offset": {
            "loc": [holo.translation.x, holo.translation.y, holo.translation.z],
            "rot": [holo.rotation.rotator().pitch, holo.rotation.rotator().yaw, holo.rotation.rotator().roll],
            "scale": [holo.scale3d.x, holo.scale3d.y, holo.scale3d.z]
        }
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_all_weapons.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ALL_WEAPONS_DONE")
