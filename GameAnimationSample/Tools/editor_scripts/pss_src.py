import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
for n in ["PSS_Walk", "PSS_Slash"]:
    s = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/All/" + n)
    k = AL.get_num_frames(s)
    names = [str(x) for x in AL.get_animation_track_names(s)][:6]
    hips = [x for x in AL.get_animation_track_names(s) if "Hips" in str(x)]
    h = str(hips[0]) if hips else "Hips"
    path = [str(x) for x in AL.find_bone_path_to_root(s, h)]
    vals = []
    for i in range(0, k + 1, 2):
        t = AL.get_bone_pose_for_frame(s, h, i, False).translation
        vals.append("(%.0f,%.0f,%.0f)" % (t.x, t.y, t.z))
    print("PSSS", n, "tracks", names, "hips", h, "path", path, " ".join(vals))
    r = AL.get_bone_pose_for_frame(s, path[-1], 0, False)
    print("PSSS root", path[-1], r.translation, r.rotation.rotator(), r.scale3d)
