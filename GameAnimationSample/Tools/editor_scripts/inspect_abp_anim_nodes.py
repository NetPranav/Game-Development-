import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

nodes = unreal.AnimationBlueprintLibrary.get_nodes_of_class(abp, unreal.AnimGraphNode_Base)
out["total_nodes"] = len(nodes)
node_info = []
for n in nodes:
    info = {
        "class": n.get_class().get_name(),
        "name": n.get_name()
    }
    # Check node properties
    for p in ["node", "Node", "RetargetAsset", "IKRetargeterAsset", "RetargetProfile"]:
        try:
            val = n.get_editor_property(p)
            info[p] = str(val)[:200]
        except Exception:
            pass
    node_info.append(info)
out["nodes"] = node_info

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_anim_nodes.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_ANIM_NODES_DONE")
