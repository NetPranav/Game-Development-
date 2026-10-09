import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")

out = {}

# Find graphs
graphs = abp.get_editor_property("function_graphs")
for g in graphs:
    gname = g.get_name()
    if "UpdateRetargetProfile" in gname:
        nodes_info = []
        for n in g.get_editor_property("nodes"):
            node_desc = {
                "class": n.get_class().get_name(),
                "name": n.get_name(),
                "title": str(n.get_node_title(unreal.NodeTitleType.FULL_TITLE)) if hasattr(n, "get_node_title") else ""
            }
            # check pins
            pins = []
            for p in n.get_editor_property("pins"):
                p_dir = str(p.get_editor_property("direction"))
                p_name = str(p.get_editor_property("pin_name"))
                p_type = str(p.get_editor_property("pin_type").pin_category)
                pins.append(f"{p_name}({p_dir},{p_type})")
            node_desc["pins"] = pins
            nodes_info.append(node_desc)
        out[gname] = nodes_info

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_graph_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_GRAPH_DETAILS_DONE")
