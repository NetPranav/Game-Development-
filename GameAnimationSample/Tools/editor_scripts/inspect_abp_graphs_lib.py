import unreal, json

bp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
graphs = unreal.BlueprintEditorLibrary.list_graphs(bp)

gnames = [g.get_name() for g in graphs]
print("All graph names:", gnames)

out = {}
for g in graphs:
    gname = g.get_name()
    if "Retarget" not in gname and "Update" not in gname:
        continue
    try:
        nodes = g.get_editor_property("nodes")
    except Exception as e:
        print(f"Skipping {gname}: {e}")
        continue
    nodes_info = []
    for n in nodes:
        pins_data = []
        for p in n.get_editor_property("pins"):
            linked = []
            for lp in p.get_editor_property("linked_to"):
                linked.append(f"{lp.get_owning_node().get_name()}.{lp.get_editor_property('pin_name')}")
            pins_data.append({
                "name": str(p.get_editor_property("pin_name")),
                "dir": str(p.get_editor_property("direction")),
                "linked": linked,
                "def": str(p.get_editor_property("default_value")),
            })
        node_info = {
            "name": n.get_name(),
            "class": n.get_class().get_name(),
            "pins": pins_data
        }
        if hasattr(n, "function_reference"):
            fr = n.get_editor_property("function_reference")
            node_info["func_member"] = str(fr.get_editor_property("member_name"))
        nodes_info.append(node_info)
    out[gname] = nodes_info

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_update_retarget_profile_graph.json", "w") as f:
    json.dump({"gnames": gnames, "graphs": out}, f, indent=2)

print("INSPECT_UPDATE_RETARGET_PROFILE_GRAPH_DONE")
