import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}
for i in range(rc.get_num_retarget_ops()):
    name = rc.get_op_name(i)
    ctrl = rc.get_op_controller(i)
    out[str(name)] = [m for m in dir(ctrl) if not m.startswith("_")]

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_op_ctrls.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_OP_CTRLS_DONE")
