import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

cast_nodes = abp.get_nodes_of_class(unreal.K2Node_DynamicCast)
out["cast_nodes"] = []
for n in cast_nodes:
    target_type = n.get_editor_property("target_type") if hasattr(n, "get_editor_property") else None
    out["cast_nodes"].append({
        "name": n.get_name(),
        "graph": n.get_graph().get_name() if n.get_graph() else None,
        "target_type": str(target_type)
    })

call_nodes = abp.get_nodes_of_class(unreal.K2Node_CallFunction)
out["call_nodes"] = []
for n in call_nodes:
    func_name = str(n.get_editor_property("function_reference").get_editor_property("member_name")) if hasattr(n, "get_editor_property") else ""
    if "Setting" in func_name or "Retarget" in func_name:
        out["call_nodes"].append({
            "name": n.get_name(),
            "graph": n.get_graph().get_name() if n.get_graph() else None,
            "func": func_name
        })

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_cast_nodes.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_CAST_NODES_DONE")
