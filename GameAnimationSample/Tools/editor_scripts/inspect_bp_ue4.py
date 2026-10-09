import unreal, json

bp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin")
out = {}

# Check construction script or functions on BP_UE4_Mannequin
cdo = unreal.get_default_object(bp.generated_class())
for p in dir(cdo):
    if not p.startswith("_"):
        try:
            val = cdo.get_editor_property(p)
            if val is not None and str(val) != "None" and str(val) != "()":
                out[p] = str(val)[:200]
        except Exception:
            pass

# Also check child actor component or skeletal mesh on BP_UE4_Mannequin
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sub.k2_gather_subobject_data_for_blueprint(bp)
comps = []
for h in handles:
    d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
    if o:
        item = {"name": o.get_name(), "class": o.get_class().get_name()}
        if isinstance(o, unreal.SkeletalMeshComponent):
            item["mesh"] = str(o.get_editor_property("skeletal_mesh_asset"))
            item["anim_class"] = str(o.get_editor_property("anim_class"))
        comps.append(item)
out["comps"] = comps

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_bp_ue4.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_BP_UE4_DONE")
