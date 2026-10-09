import unreal
import builtins
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
# Read-only: component-space Z of pelvis / feet over time for the player's montages (UEFN skeleton).
AL = unreal.AnimationLibrary
MONTAGES = ["AM_P_UAL_L1", "AM_P_UAL_L2", "AM_P_UAL_L3", "AM_P_SwordDash", "AM_P_Knockback", "AM_P_GuardBreak", "AM_P_SwordBlock", "AM_P_Slide"]

def seqs_of(m):
    out = []
    for t in m.get_editor_property("slot_anim_tracks"):
        for seg in t.get_editor_property("anim_track").get_editor_property("anim_segments"):
            out.append(seg.get_editor_property("anim_reference"))
    return out

def cs_z(anim, bone, t):
    path = AL.find_bone_path_to_root(anim, bone)  # bone ... root
    xf = unreal.Transform()
    for b in reversed(path):
        local = AL.get_bone_pose_for_time(anim, b, t, False)
        xf = local.multiply(xf) if hasattr(local, "multiply") else local * xf
    return xf.translation.z

def report(anim, label):
    if not anim:
        return
    L = anim.get_play_length()
    rows = []
    for i in range(9):
        t = L * i / 8.0
        try:
            rows.append("%.0f/%.0f/%.0f" % (cs_z(anim, "pelvis", t), cs_z(anim, "foot_l", t), cs_z(anim, "foot_r", t)))
        except Exception as e:
            rows.append("err:%s" % e)
            break
    print("HGT %-18s %-40s len=%.2f rm=%s  pelvis/footL/footR: %s" % (label, anim.get_name(), L, anim.get_editor_property("enable_root_motion"), " ".join(rows)))

for n in MONTAGES:
    m = unreal.load_asset("/Game/RR/Player/" + n)
    if not m:
        print("HGT missing", n); continue
    print("HGT", n, "slots:", [str(t.get_editor_property("slot_name")) for t in m.get_editor_property("slot_anim_tracks")])
    for s in seqs_of(m):
        report(s, n)

# reference: a locomotion idle on the same skeleton
ar = unreal.AssetRegistryHelpers.get_asset_registry()
flt = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")], package_paths=["/Game/Characters/UEFN_Mannequin/Animations"], recursive_paths=True)
n = 0
for a in ar.get_assets(flt):
    nm = str(a.asset_name)
    if "Idle" in nm and n < 2:
        report(a.get_asset(), "REF"); n += 1
print("HGT_DONE")
