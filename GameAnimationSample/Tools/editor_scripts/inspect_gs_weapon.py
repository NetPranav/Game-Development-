import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets(unreal.ARFilter(recursive_paths=True, package_paths=["/Game"], class_paths=[unreal.TopLevelAssetPath("/Script/AAACombat", "AAAWeaponSettings")])):
    if "Greystone" not in str(a.asset_name): continue
    da = a.get_asset()
    an = da.get_editor_property("animations")
    for k in ("equip_animation", "unequip_animation", "block_montage", "parry_montage"):
        try: m = an.get_editor_property(k)
        except Exception as e: print("GSW err", k, e); continue
        if not m: print("GSW", da.get_name(), k, None); continue
        secs = []
        for s in m.get_editor_property("composite_sections"):
            secs.append("%s->%s" % (s.get_editor_property("section_name"), s.get_editor_property("next_section_name")))
        print("GSW", da.get_name(), k, m.get_path_name(), "len %.2f" % m.get_play_length(), "slots", [str(t.get_editor_property("slot_name")) for t in m.get_editor_property("slot_anim_tracks")], "sections", secs)
