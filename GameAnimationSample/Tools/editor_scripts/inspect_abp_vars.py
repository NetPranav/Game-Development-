import unreal, json

out = {}

# Check BP_UE4_Mannequin
bp_ue4 = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin")
# Check variables on bp_ue4
out["ue4_vars"] = [str(v.get_name()) for v in bp_ue4.get_editor_property("new_variables")] if hasattr(bp_ue4, "new_variables") else []

# Check BP_RobotScout
bp_scout = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
out["scout_vars"] = [str(v.get_name()) for v in bp_scout.get_editor_property("new_variables")] if hasattr(bp_scout, "new_variables") else []

# Check ABP_GenericRetarget variables
abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out["abp_vars"] = [str(v.get_name()) for v in abp.get_editor_property("new_variables")] if hasattr(abp, "new_variables") else []

# In abp, what are default values of variables?
cdo = unreal.get_default_object(abp.generated_class())
var_defaults = {}
for v_name in ["IKRetargeter", "RetargetAsset", "RetargetProfile", "TargetSkeleton", "SourceMeshComponent", "TargetMeshComponent"]:
    try:
        var_defaults[v_name] = str(cdo.get_editor_property(v_name))
    except Exception as e:
        var_defaults[v_name] = str(e)
out["abp_var_defaults"] = var_defaults

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_vars.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_VARS_DONE")
