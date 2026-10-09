import unreal, json

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}
for S in [unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET]:
    s_name = "SOURCE" if S == unreal.RetargetSourceOrTarget.SOURCE else "TARGET"
    offsets = {}
    for b in ["upperarm_l", "upperarm_r", "clavicle_l", "clavicle_r"]:
        try:
            q = rc.get_rotation_offset_for_retarget_pose_bone(b, S)
            r = q.rotator()
            offsets[b] = [r.pitch, r.yaw, r.roll]
        except Exception as e:
            offsets[b] = str(e)
    out[s_name] = offsets

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ue4_rtg_pose2.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_UE4_RTG_POSE2_DONE")
