import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
cdo = unreal.get_default_object(abp.generated_class())

out = {}
# List all properties on CDO
for p in dir(cdo):
    if not p.startswith("_"):
        try:
            val = cdo.get_editor_property(p)
            out[p] = str(val)[:300]
        except Exception:
            pass

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_all_props.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_ALL_PROPS_DONE")
