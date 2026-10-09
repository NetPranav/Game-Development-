import unreal, json

eal = unreal.EditorAssetLibrary

src_rig = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/IK_ProSwordShield")
tgt_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")

out = {}

def get_chains(rig):
    if not rig: return "NOT FOUND"
    c = unreal.IKRigController.get_controller(rig)
    chains = c.get_retarget_chains()
    res = {}
    for ch in chains:
        name = str(ch.get_editor_property("chain_name"))
        start = str(ch.get_editor_property("start_bone"))
        end = str(ch.get_editor_property("end_bone"))
        res[name] = {"start": start, "end": end}
    return res

out["source_rig_chains"] = get_chains(src_rig)
out["target_rig_chains"] = get_chains(tgt_rig)

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rig_chains.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_RIGS_DONE")
