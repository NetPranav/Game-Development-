import unreal, json

out = {}

for rtg_path in [
    "/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN",
    "/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin",
    "/Game/RR/ExternalAnims/UAL1/RTG_UAL1_to_UEFN",
    "/Game/RR/ExternalAnims/UAL2/Retarget/RTG_UAL2_to_UEFN"
]:
    rtg = unreal.load_asset(rtg_path)
    if not rtg:
        continue
    rc = unreal.IKRetargeterController.get_controller(rtg)
    name = rtg.get_name()
    poses = {}
    for S in [unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET]:
        s_name = "SOURCE" if S == unreal.RetargetSourceOrTarget.SOURCE else "TARGET"
        pose_names = rc.get_retarget_poses(S)
        curr = rc.get_current_retarget_pose_name(S)
        poses[s_name] = {
            "current": str(curr),
            "all_poses": [str(p) for p in pose_names]
        }
    out[name] = poses

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_rtg_poses.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_RTG_POSES_DONE")
