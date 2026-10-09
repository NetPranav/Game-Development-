import unreal
# Repairs RT_PSS_* (Pro Sword & Shield -> UEFN): the retargeter's pelvis op leaks the Mixamo hips' forward travel into
# height (walk pelvis 97 -> 20 cm mid-step, slashes below the floor) because the Mixamo hips bone is rotated 90 deg.
# Rebuild the pelvis translation from the source hips (both skeletons: Z up, +Y forward), scaled to the UEFN pelvis
# height, then remove the net horizontal travel (linear trend) so loops and attacks play in place over the capsule.
# Leg/arm FK from the retargeter is kept. Idempotent (always rebuilt from the source).
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
SRC = "/Game/RR/ExternalAnims/ProSwordShield/All/"
src_skel = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/SKM_ProSwordShield_Base").get_editor_property("skeleton")
tgt_ref = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin").get_reference_pose()
src_h = src_skel.get_reference_pose().get_ref_bone_pose("Hips").translation
tgt_h = tgt_ref.get_ref_bone_pose("pelvis").translation
SCALE = tgt_h.z / max(1.0, src_h.z)
print("PSSR scale", SCALE, "src hips", src_h, "tgt pelvis", tgt_h)

def V(t): return unreal.Vector(t.x, t.y, t.z)

for a in ar.get_assets(unreal.ARFilter(package_paths=["/Game/RR/Player/Anims/PSS"], class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")])):
    seq = a.get_asset()
    name = str(a.asset_name)
    src = unreal.load_asset(SRC + name[3:])
    if not src:
        print("PSSR no source for", name); continue
    n = AL.get_num_frames(seq) + 1
    L = max(0.001, seq.get_play_length())
    SL = src.get_play_length()
    P = [AL.get_bone_pose_for_frame(seq, "pelvis", i, False) for i in range(n)]
    H = []
    for i in range(n):
        t = (i / max(1, n - 1)) * SL
        H.append(AL.get_bone_pose_for_time(src, "Hips", t, False).translation)
    first, last = H[0], H[-1]
    pos = []
    for i, h in enumerate(H):
        f = i / max(1, n - 1)
        x = (h.x - first.x - (last.x - first.x) * f) * SCALE + tgt_h.x
        y = (h.y - first.y - (last.y - first.y) * f) * SCALE + tgt_h.y
        z = h.z * SCALE
        pos.append(unreal.Vector(x, y, z))
    c = seq.get_editor_property("controller")
    ok = c.set_bone_track_keys("pelvis", pos, [p.rotation for p in P], [V(p.scale3d) for p in P], False)
    seq.set_editor_property("enable_root_motion", False)
    unreal.EditorAssetLibrary.save_loaded_asset(seq)
    zs = [p.z for p in pos]
    print("PSSR %-30s ok %s  z %.0f..%.0f  removed travel (%.0f, %.0f)" % (name, ok, min(zs), max(zs), (last.x - first.x) * SCALE, (last.y - first.y) * SCALE))
print("PSSR_DONE")
