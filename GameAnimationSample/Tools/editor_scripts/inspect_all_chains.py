import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

tgt_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
src_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.SOURCE)

tgt_rc = unreal.IKRigController.get_controller(tgt_rig)
src_rc = unreal.IKRigController.get_controller(src_rig)

out = {
    "src_root": str(src_rc.get_retarget_root()),
    "tgt_root": str(tgt_rc.get_retarget_root()),
    "src_chains": {},
    "tgt_chains": {},
    "mappings": {}
}

for c in src_rc.get_retarget_chains():
    d = c.to_dict()
    out["src_chains"][str(d["chain_name"])] = {
        "ik_goal": str(d["ik_goal_name"])
    }

for c in tgt_rc.get_retarget_chains():
    d = c.to_dict()
    name = str(d["chain_name"])
    src_chain = str(rc.get_source_chain(name))
    out["tgt_chains"][name] = {
        "ik_goal": str(d["ik_goal_name"])
    }
    out["mappings"][name] = src_chain

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_all_chains.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ALL_CHAINS_DONE")
