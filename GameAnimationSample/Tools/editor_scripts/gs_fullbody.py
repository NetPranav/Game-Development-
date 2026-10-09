import unreal
# Greystone_AnimBlueprint only has a "FullBody" slot node: montages on DefaultSlot never show on the boss.
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets(unreal.ARFilter(package_paths=["/Game/RR/Greystone"], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage")])):
    m = a.get_asset()
    tracks = m.get_editor_property("slot_anim_tracks")
    changed = False
    for t in tracks:
        if str(t.get_editor_property("slot_name")) != "FullBody":
            t.set_editor_property("slot_name", "FullBody"); changed = True
    if changed:
        m.set_editor_property("slot_anim_tracks", tracks)
        unreal.EditorAssetLibrary.save_loaded_asset(m)
    print("GSFB", m.get_name(), [str(t.get_editor_property("slot_name")) for t in m.get_editor_property("slot_anim_tracks")])
