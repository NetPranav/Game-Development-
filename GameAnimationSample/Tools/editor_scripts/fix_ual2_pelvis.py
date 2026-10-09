import unreal
# The UAL2 -> UEFN retarget left the pelvis track at ~0 cm and put the hip height on the thighs, so every UAL2 montage drew the
# torso at ground level ("sinks into the ground"). Move the shared thigh offset back onto the pelvis: legs keep their exact
# world positions, the spine/arms/head go back up. Idempotent (skips sequences whose pelvis is already high).
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()

def V(t): return unreal.Vector(t.x, t.y, t.z)

def get_controller(seq):
    for fn in (lambda s: s.get_editor_property("controller"),
               lambda s: s.controller,
               lambda s: s.get_data_model_interface().get_controller(),
               lambda s: s.get_editor_property("data_model_interface").get_controller()):
        try:
            c = fn(seq)
            if c:
                return c
        except Exception:
            pass
    return None

flt = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")], package_paths=["/Game/RR/Player/Anims", "/Game/RR/NPC/Anims"], recursive_paths=True)
fixed = 0
for a in ar.get_assets(flt):
    if not str(a.asset_name).startswith(("RT_UAL2_", "RT_UAL1_")):
        continue
    seq = a.get_asset()
    sk = seq.get_editor_property("skeleton")
    ref = sk.get_reference_pose()
    refL, refR = ref.get_ref_bone_pose("thigh_l"), ref.get_ref_bone_pose("thigh_r")
    n = AL.get_num_frames(seq) + 1
    t0 = AL.get_bone_pose_for_frame(seq, "thigh_l", 0, False)
    if (V(t0.translation) - V(refL.translation)).length() < 5.0:
        print("FIXP skip (already ok)", seq.get_name(), AL.get_bone_pose_for_frame(seq, "pelvis", 0, False).translation)
        continue
    P, L, R = [], [], []
    for i in range(n):
        P.append(AL.get_bone_pose_for_frame(seq, "pelvis", i, False))
        L.append(AL.get_bone_pose_for_frame(seq, "thigh_l", i, False))
        R.append(AL.get_bone_pose_for_frame(seq, "thigh_r", i, False))
    pp, pr, ps, lp, lr, ls, rp, rr, rs = [], [], [], [], [], [], [], [], []
    for i in range(n):
        off = (V(L[i].translation) - V(refL.translation) + V(R[i].translation) - V(refR.translation)) * 0.5
        pp.append(V(P[i].translation) + P[i].rotation.rotate_vector(off))
        pr.append(P[i].rotation); ps.append(V(P[i].scale3d))
        lp.append(V(L[i].translation) - off); lr.append(L[i].rotation); ls.append(V(L[i].scale3d))
        rp.append(V(R[i].translation) - off); rr.append(R[i].rotation); rs.append(V(R[i].scale3d))
    c = get_controller(seq)
    if not c:
        print("FIXP no controller for", seq.get_name()); continue
    c.open_bracket(unreal.Text("fix pelvis"), False) if hasattr(c, "open_bracket") else None
    ok = c.set_bone_track_keys("pelvis", pp, pr, ps, False) and c.set_bone_track_keys("thigh_l", lp, lr, ls, False) and c.set_bone_track_keys("thigh_r", rp, rr, rs, False)
    c.close_bracket(False) if hasattr(c, "close_bracket") else None
    unreal.EditorAssetLibrary.save_loaded_asset(seq)
    q = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
    print("FIXP", seq.get_name(), "frames", n, "ok", ok, "pelvis0 ->", q.translation)
    fixed += 1
print("FIXP_DONE", fixed)
