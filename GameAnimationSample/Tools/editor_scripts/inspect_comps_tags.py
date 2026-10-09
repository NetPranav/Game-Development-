import unreal, json

sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)

def get_bp_comps(path):
    bp = unreal.load_asset(path)
    res = []
    for h in sub.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o:
            tags = [str(t) for t in o.get_editor_property("component_tags")] if hasattr(o, "component_tags") else []
            res.append({
                "class": o.get_class().get_name(),
                "name": o.get_name(),
                "tags": tags
            })
    return res

out = {
    "ue4_mannequin": get_bp_comps("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin"),
    "robot_scout": get_bp_comps("/Game/RR/Blueprints/BP_RobotScout")
}

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_comps_tags.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_COMPS_TAGS_DONE")
