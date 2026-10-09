import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

tgt_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
rig_c = unreal.IKRigController.get_controller(tgt_rig)
chains = rig_c.get_retarget_chains()
c = chains[0]

out = {
    "chain_dir": dir(c),
    "chain_str": str(c)
}
with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_chain_dir.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_CHAIN_DIR_DONE")
