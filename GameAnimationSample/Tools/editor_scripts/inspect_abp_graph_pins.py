import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")

out = []
for g in abp.get_editor_property("function_graphs"):
    if "UpdateRetargetProfile" in g.get_name():
        for n in g.get_editor_property("nodes"):
            pins_data = []
            for p in n.get_editor_property("pins"):
                linked = []
                for lp in p.get_editor_property("linked_to"):
                    linked.append(f"{lp.get_owning_node().get_name()}.{lp.get_editor_property('pin_name')}")
                pins_data.append({
                    "name": str(p.get_editor_property("pin_name")),
                    "direction": str(p.get_editor_property("direction")),
                    "linked_to": linked,
                    "default_value": str(p.get_editor_property("default_value")),
                    "default_text": str(p.get_editor_property("default_text_value"))
                })
            node_data = {
                "name": n.get_name(),
                "class": n.get_class().get_name(),
                "pins": pins_data
            }
            if hasattr(n, "function_reference"):
                fr = n.get_editor_property("function_reference")
                node_data["func_member_name"] = str(fr.get_editor_property("member_name"))
            if hasattr(n, "target_type"):
                node_data["target_type"] = str(n.get_editor_property("target_type"))
            out.append(node_data)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_graph_pins.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_GRAPH_PINS_DONE")
