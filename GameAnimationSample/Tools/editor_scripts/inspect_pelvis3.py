import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
for n in ["RT_UAL2_StandardSword_Regular_A", "RT_UAL2_StandardSword_Regular_B", "RT_UAL2_StandardSword_Regular_C", "RT_UAL2_StandardSword_Dash_RM", "RT_UAL2_StandardHit_Knockback"]:
    a = unreal.load_asset("/Game/RR/Player/Anims/" + n)
    k = AL.get_num_frames(a)
    out = []
    for i in range(0, k + 1, max(1, k // 6)):
        t = AL.get_bone_pose_for_frame(a, "pelvis", i, False).translation
        out.append("%d:(%.0f,%.0f,%.0f)" % (i, t.x, t.y, t.z))
    th = AL.get_bone_pose_for_frame(a, "thigh_l", 0, False).translation
    print("P3", n, " ".join(out), "thighL0=(%.1f,%.1f,%.1f)" % (th.x, th.y, th.z))
src = unreal.load_asset("/Game/RR/ExternalAnims/UAL2/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Regular_B")
k = AL.get_num_frames(src)
print("P3 SRC_B", " ".join("%d:(%.0f,%.0f,%.0f)" % ((i,) + tuple(getattr(AL.get_bone_pose_for_frame(src, "pelvis", i, False).translation, c) for c in "xyz")) for i in range(0, k + 1, max(1, k // 6))))
