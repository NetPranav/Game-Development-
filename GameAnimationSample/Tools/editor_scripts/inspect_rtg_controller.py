import unreal, json

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {
    "rc_methods": [m for m in dir(rc) if not m.startswith("_")]
}

# Can we check what op classes exist?
out["op_classes"] = []
for name in dir(unreal):
    if "IKRetarget" in name and "Op" in name:
        out["op_classes"].append(name)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_rtg_controller.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_RTG_CONTROLLER_DONE")
