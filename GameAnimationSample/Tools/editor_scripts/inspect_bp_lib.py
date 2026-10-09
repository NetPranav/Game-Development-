import unreal, json

out = {
    "bp_lib": [m for m in dir(unreal.BlueprintEditorLibrary) if not m.startswith("_")] if hasattr(unreal, "BlueprintEditorLibrary") else []
}

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_bp_lib.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BP_LIB_DONE")
