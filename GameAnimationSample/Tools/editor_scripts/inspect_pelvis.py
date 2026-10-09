import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()

def dump(anim, label):
    L = anim.get_play_length()
    names = AL.get_animation_track_names(anim) if hasattr(AL, "get_animation_track_names") else []
    pel = [n for n in names if "pelvis" in str(n).lower() or "hips" in str(n).lower()]
    bone = str(pel[0]) if pel else "pelvis"
    vals = []
    for i in range(4):
        t = L * i / 3.0
        p = AL.get_bone_pose_for_time(anim, bone, t, False)
        vals.append("(%.2f,%.2f,%.2f)" % (p.translation.x, p.translation.y, p.translation.z))
    path = [str(x) for x in AL.find_bone_path_to_root(anim, bone)]
    print("PEL %s %s bone=%s path=%s %s" % (label, anim.get_path_name(), bone, path, " ".join(vals)))
    sk = anim.get_editor_property("skeleton")
    try:
        ref = sk.get_reference_pose() if hasattr(sk, "get_reference_pose") else None
        print("PEL   skeleton", sk.get_path_name(), "ref", ref.get_ref_bone_pose(bone) if ref else "-")
    except Exception as e:
        print("PEL   skel err", e)

for path in ["/Game/RR/ExternalAnims/UAL2"]:
    flt = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")], package_paths=[path], recursive_paths=True)
    for a in ar.get_assets(flt):
        n = str(a.asset_name)
        if "Regular_A" in n or "Shield_Break" in n:
            dump(a.get_asset(), "SRC/RT")
m = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Animations")
flt = unreal.ARFilter(class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")], package_paths=["/Game/Characters/UEFN_Mannequin/Animations"], recursive_paths=True)
for a in ar.get_assets(flt):
    if "Idle" in str(a.asset_name) and "Crouch" not in str(a.asset_name):
        dump(a.get_asset(), "REF"); break
print("PEL_DONE")
