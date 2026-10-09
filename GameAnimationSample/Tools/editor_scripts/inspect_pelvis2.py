import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
AL = unreal.AnimationLibrary
for p in ["/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_B", "/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A", "/Game/Characters/UEFN_Mannequin/Animations/Idle/M_Neutral_Idle_turn_left"]:
    a = unreal.load_asset(p)
    for b in ["root", "pelvis", "thigh_l", "calf_l", "foot_l"]:
        x = AL.get_bone_pose_for_time(a, b, 0.0, False)
        t, r = x.translation, x.rotation.rotator()
        print("PEL2 %s %-8s t=(%.1f,%.1f,%.1f) r=(%.0f,%.0f,%.0f)" % (a.get_name(), b, t.x, t.y, t.z, r.pitch, r.yaw, r.roll))
    print("PEL2 skel", a.get_editor_property("skeleton").get_path_name())
