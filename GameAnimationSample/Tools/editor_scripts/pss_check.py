import unreal, math
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
ref = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin").get_reference_pose()
refL = ref.get_ref_bone_pose("thigh_l").translation
for a in sorted(ar.get_assets(unreal.ARFilter(package_paths=["/Game/RR/Player/Anims/PSS"], class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")])), key=lambda x: str(x.asset_name)):
    s = a.get_asset()
    k = AL.get_num_frames(s)
    P = [AL.get_bone_pose_for_frame(s, "pelvis", i, False).translation for i in range(0, k + 1)]
    R = [AL.get_bone_pose_for_frame(s, "root", i, False).translation for i in (0, k)]
    th = AL.get_bone_pose_for_frame(s, "thigh_l", 0, False).translation
    zs = [p.z for p in P]
    dx, dy = P[-1].x - P[0].x, P[-1].y - P[0].y
    mx = max(math.hypot(p.x - P[0].x, p.y - P[0].y) for p in P)
    thoff = math.sqrt((th.x - refL.x) ** 2 + (th.y - refL.y) ** 2 + (th.z - refL.z) ** 2)
    print("PSSC %-34s len %.2f  pz min %3.0f med %3.0f max %3.0f  drift (%5.0f,%5.0f) maxdev %4.0f  root (%.0f,%.0f)->(%.0f,%.0f) thighoff %.0f" % (
        str(a.asset_name), s.get_play_length(), min(zs), sorted(zs)[len(zs) // 2], max(zs), dx, dy, mx, R[0].x, R[0].y, R[1].x, R[1].y, thoff))
