import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
gen = abp.generated_class()
cdo = unreal.get_default_object(gen)

out = {
    "cdo_props": {}
}

for prop in ["IKRetargeter", "ik_retargeter", "RetargetProfile", "TargetMesh"]:
    try:
        val = cdo.get_editor_property(prop)
        out["cdo_props"][prop] = str(val)
    except Exception as e:
        out["cdo_props"][prop + "_err"] = str(e)

# Let's inspect all properties on CDO
all_props = {}
for p in dir(cdo):
    if not p.startswith("_"):
        try:
            v = cdo.get_editor_property(p)
            if v is not None:
                all_props[p] = str(v)
        except Exception:
            pass
out["all_cdo_props"] = all_props

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_error_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_ERROR_DETAILS_DONE")
