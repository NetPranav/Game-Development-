import unreal, json

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}
for S in [unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET]:
    s_name = "SOURCE" if S == unreal.RetargetSourceOrTarget.SOURCE else "TARGET"
    pose = rc.get_current_retarget_pose_name(S)
    # Check all bone rotation offsets in this pose
    offsets = {}
    # Check bones
    rig = rc.get_ik_rig(S)
    rig_c = unreal.IKRigController.get_controller(rig)
    # Let's inspect rc methods for pose bone offsets
    for b in ["upperarm_l", "upperarm_r", "clavicle_l", "clavicle_r", "LeftArm", "RightArm"]:
        try:
            rot = rc.get_rotation_offset_for_retarget_pose_bone(b, S)
            offsets[b] = [rot.pitch, rot.yaw, rot.roll]
        except Exception as e:
            offsets[b] = str(e)
    out[s_name] = offsets

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ue4_rtg_pose.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_UE4_RTG_POSE_DONE")
