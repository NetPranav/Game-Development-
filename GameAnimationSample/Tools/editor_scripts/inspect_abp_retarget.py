import unreal, json

out = {}

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
if abp:
    cdo = unreal.get_default_object(abp.generated_class())
    out["abp_props"] = {}
    for p in ["RetargetAsset", "IKRetargeter", "RetargetProfile", "TargetSkeleton", "SourceMeshComponent", "TargetMeshComponent"]:
        try:
            out["abp_props"][p] = str(cdo.get_editor_property(p))
        except Exception as e:
            out["abp_props"][p] = f"ERR: {e}"

bp_scout = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
if bp_scout:
    cdo_s = unreal.get_default_object(bp_scout.generated_class())
    out["scout_props"] = {}
    for p in ["RetargetAsset", "RetargetProfile", "Retargeter"]:
        try:
            out["scout_props"][p] = str(cdo_s.get_editor_property(p))
        except Exception as e:
            out["scout_props"][p] = f"ERR: {e}"

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_retarget.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_DONE")
