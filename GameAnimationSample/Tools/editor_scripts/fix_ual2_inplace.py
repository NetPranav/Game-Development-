import unreal
# UAL2 attacks carry their forward travel on the pelvis (root track is static): Regular_A +84 cm, Regular_C +230 cm,
# Sword_Dash +370 cm. The mesh ran ahead of the capsule (through slopes) and snapped back when the montage ended.
# Make them in-place on the forward axis (+Y on the UEFN mannequin); the melee ability's Mover lunge moves the pawn.
# Run after fix_ual2_pelvis.py. Idempotent.
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
REST_Y = -8.5      # pelvis Y of the UAL2 idle/guard pose
DRIFT_LIMIT = 30.0

def V(t): return unreal.Vector(t.x, t.y, t.z)

flt = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")], package_paths=["/Game/RR/Player/Anims", "/Game/RR/NPC/Anims"], recursive_paths=True)
for a in ar.get_assets(flt):
    if not str(a.asset_name).startswith(("RT_UAL2_", "RT_UAL1_")):
        continue
    seq = a.get_asset()
    n = AL.get_num_frames(seq) + 1
    P = [AL.get_bone_pose_for_frame(seq, "pelvis", i, False) for i in range(n)]
    ys = [p.translation.y for p in P]
    drift = max(abs(y - REST_Y) for y in ys)
    if drift < DRIFT_LIMIT:
        print("INPLACE skip", seq.get_name(), "drift %.0f" % drift)
        continue
    pos = []
    for p in P:
        t = V(p.translation)
        pos.append(unreal.Vector(t.x, REST_Y, t.z))
    c = seq.get_editor_property("controller")
    ok = c.set_bone_track_keys("pelvis", pos, [p.rotation for p in P], [V(p.scale3d) for p in P], False)
    unreal.EditorAssetLibrary.save_loaded_asset(seq)
    print("INPLACE", seq.get_name(), "drift was %.0f cm" % drift, "ok", ok)
print("INPLACE_DONE")
