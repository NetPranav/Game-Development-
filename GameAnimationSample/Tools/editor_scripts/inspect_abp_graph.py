import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

# Inspect graph or functions on abp
try:
    for g in abp.get_editor_property("function_graphs"):
        if "UpdateRetargetProfile" in g.get_name():
            nodes = []
            for n in g.get_editor_property("nodes"):
                nodes.append({
                    "name": n.get_name(),
                    "class": n.get_class().get_name(),
                    "title": str(n.get_node_title(unreal.NodeTitleType.FULL_TITLE)) if hasattr(n, 'get_node_title') else None
                })
            out["nodes"] = nodes
except Exception as e:
    out["err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_graph.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_GRAPH_DONE")
