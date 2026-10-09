import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
for n in ["RT_PSS_Walk", "RT_PSS_Slash", "RT_PSS_Attack", "RT_PSS_Strafe", "RT_PSS_Slash5"]:
    s = unreal.load_asset("/Game/RR/Player/Anims/PSS/" + n)
    k = AL.get_num_frames(s)
    zs = ["%.0f" % AL.get_bone_pose_for_frame(s, "pelvis", i, False).translation.z for i in range(k + 1)]
    print("PSSF", n, k, " ".join(zs))
