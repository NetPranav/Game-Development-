import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

# Check all functions in generated class
cls = abp.generated_class()
funcs = []
for p in dir(cls):
    if not p.startswith("_"):
        funcs.append(p)
out["class_members"] = funcs

# Check variables on CDO
cdo = unreal.get_default_object(cls)
props = {}
for p in ["IKRetargeter", "RetargetProfile", "retarget_asset", "SourceMeshComponent", "TargetMeshComponent"]:
    try:
        props[p] = str(cdo.get_editor_property(p))
    except Exception as e:
        props[p] = str(e)
out["cdo_props"] = props

# Check BP_RobotScout CDO properties
scout_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
scout_cdo = unreal.get_default_object(scout_bp.generated_class())
scout_props = {}
for p in dir(scout_cdo):
    if not p.startswith("_"):
        try:
            val = scout_cdo.get_editor_property(p)
            if val is not None and str(val) != "None" and str(val) != "()":
                scout_props[p] = str(val)[:200]
        except Exception:
            pass
out["scout_cdo_props"] = scout_props

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_error.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_ERROR_DONE")
