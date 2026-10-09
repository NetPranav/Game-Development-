import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")

out = {
    "abp_props": [p for p in dir(abp) if not p.startswith("_")]
}

with open(unreal.Paths.project_saved_dir() + "RRTools/abp_dir.json", "w") as f:
    json.dump(out, f, indent=2)

print("ABP_DIR_DONE")
