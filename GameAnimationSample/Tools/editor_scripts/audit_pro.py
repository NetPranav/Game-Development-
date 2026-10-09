import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
an = da.get_editor_property("animations")
for k in ("light_attack_combos", "heavy_attack_combos", "block_montage", "parry_montage", "equip_animation", "unequip_animation"):
    try:
        v = an.get_editor_property(k)
        print("AUD DA", k, [x.get_name() for x in v] if isinstance(v, (list, unreal.Array)) else (v.get_name() if v else None))
    except Exception as e: print("AUD DA err", k, e)
bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name():
        r = o.get_editor_property("hit_reactions")
        print("AUD HR", {k: (r.get_editor_property(k).get_name() if r.get_editor_property(k) else None) for k in ("front","back","left","right","stagger","death","knockdown","get_up")})
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets(unreal.ARFilter(package_paths=["/Game/RR/Player"], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimMontage")])):
    m = a.get_asset()
    segs = []
    for t in m.get_editor_property("slot_anim_tracks"):
        for sg in t.get_editor_property("anim_track").get_editor_property("anim_segments"):
            r = sg.get_editor_property("anim_reference")
            segs.append((str(t.get_editor_property("slot_name")), r.get_name() if r else None, r.get_editor_property("skeleton").get_name() if r else None))
    print("AUD M", m.get_name(), "len %.2f rate %.2f" % (m.get_play_length(), m.get_editor_property("rate_scale")), segs)
for a in ar.get_assets(unreal.ARFilter(package_paths=["/Game/RR/Player/Anims"], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")])):
    n = str(a.asset_name)
    if "Pro" not in n: continue
    s = a.get_asset()
    k = AL.get_num_frames(s)
    p = [AL.get_bone_pose_for_frame(s, "pelvis", i, False).translation for i in (0, k // 2, k)]
    th = AL.get_bone_pose_for_frame(s, "thigh_l", 0, False).translation
    print("AUD S %-26s %-14s len %.2f pelvis %s thighL0 (%.0f,%.0f,%.0f)" % (n, s.get_editor_property("skeleton").get_name(), s.get_play_length(), " ".join("(%.0f,%.0f,%.0f)" % (v.x, v.y, v.z) for v in p), th.x, th.y, th.z))
