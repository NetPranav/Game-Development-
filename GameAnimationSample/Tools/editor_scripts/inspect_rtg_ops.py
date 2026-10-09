import unreal, json

out = {}
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
if rtg:
    rc = unreal.IKRetargeterController.get_controller(rtg)
    ops = []
    for i in range(rc.get_num_retarget_ops()):
        name = str(rc.get_op_name(i))
        enabled = rc.get_retarget_op_enabled(i)
        ctrl = rc.get_op_controller(i)
        ops.append({
            "index": i,
            "name": name,
            "enabled": enabled,
            "controller_class": str(ctrl.get_class().get_name()) if ctrl else None
        })
    out["rtg_ops"] = ops

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_rtg_ops.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_RTG_OPS_DONE")
