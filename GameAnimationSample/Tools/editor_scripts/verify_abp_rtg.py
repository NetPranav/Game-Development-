import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
cdo = unreal.get_default_object(abp.generated_class())
val = cdo.get_editor_property("IKRetargeter")

out = {
    "IKRetargeter": str(val.get_path_name()) if val else "None"
}
with open(unreal.Paths.project_saved_dir() + "RRTools/verify_abp_rtg.json", "w") as f:
    json.dump(out, f, indent=2)

print("VERIFY_SAVED")
