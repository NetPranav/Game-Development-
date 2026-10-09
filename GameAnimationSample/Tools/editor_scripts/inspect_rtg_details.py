import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET

out = {
    "source_retarget_root": str(rc.get_retarget_root(S)),
    "target_retarget_root": str(rc.get_retarget_root(T)),
    "root_settings": {
        "source_chain_for_root": str(rc.get_source_chain("Root")),
        "root_motion_source": str(rc.get_source_chain("Pelvis"))
    }
}

# Print all target chains and their current source chains in RTG
all_chains = {}
tgt_rig = rtg.get_editor_property("target_ik_rig")
tgt_c = unreal.IKRigController.get_controller(tgt_rig)
for ch in tgt_c.get_retarget_chains():
    c_name = str(ch.get_editor_property("chain_name"))
    all_chains[c_name] = str(rc.get_source_chain(c_name))

out["all_target_to_source_chains"] = all_chains

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rtg_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_RTG_DETAILS_DONE")
