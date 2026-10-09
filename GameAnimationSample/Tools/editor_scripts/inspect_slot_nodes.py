import unreal, json

abp = unreal.load_asset("/Game/Blueprints/SandboxCharacter_Mover_ABP")
out = {}

nodes = unreal.AnimationBlueprintLibrary.get_nodes_of_class(abp, unreal.AnimGraphNode_Base)
out["count"] = len(nodes)
slot_nodes = []
for n in nodes:
    cname = n.get_class().get_name()
    if "Slot" in cname or "Montage" in cname or "Layer" in cname:
        slot_nodes.append({
            "class": cname,
            "name": n.get_name()
        })
out["slot_nodes"] = slot_nodes

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/abp_slot_nodes.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_SLOT_NODES_DONE")
