import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
for n in ["Dance_Loop", "Idle_Talking_Loop", "Hit_Chest", "Hit_Head", "Roll"]:
    s = unreal.load_asset("/Game/RR/NPC/Anims/RT_UAL1_AL_StandardRig_" + n)
    k = AL.get_num_frames(s)
    ps = []
    for i in range(0, k + 1, max(1, k // 4)):
        p = AL.get_bone_pose_for_frame(s, "pelvis", i, False).translation
        ps.append("(%.0f,%.0f,%.0f)" % (p.x, p.y, p.z))
    th = AL.get_bone_pose_for_frame(s, "thigh_l", 0, False).translation
    print("CHK1", n, "len %.2f" % s.get_play_length(), "pelvis", " ".join(ps), "thighL0 (%.1f,%.1f,%.1f)" % (th.x, th.y, th.z))
