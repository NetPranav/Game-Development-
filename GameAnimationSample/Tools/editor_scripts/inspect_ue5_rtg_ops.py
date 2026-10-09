import unreal, json

rtg = unreal.load_asset("/Game/Characters/UE5_Mannequins/Rigs/RTG_UEFN_to_UE5_Mannequin")
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

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ue5_rtg_ops.json", "w") as f:
    json.dump(ops, f, indent=2)

print("INSPECT_UE5_RTG_OPS_DONE")
