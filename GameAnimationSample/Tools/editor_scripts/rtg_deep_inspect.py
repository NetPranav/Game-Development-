import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

chains = []
for c in rc.get_chain_mappings():
    chains.append({
        "target": str(c.get_editor_property("target_chain")),
        "source": str(c.get_editor_property("source_chain"))
    })

# Also check root settings in controller
root_settings = {}
try:
    rs = rc.get_root_settings()
    root_settings["copy_mode"] = str(rs.get_editor_property("copy_mode")) if hasattr(rs, "copy_mode") else "unknown"
except Exception as e:
    root_settings["error"] = str(e)

# Let's inspect target IK Rig and source IK Rig chains
src_rig = rc.get_source_ik_rig()
tgt_rig = rc.get_target_ik_rig()

src_chains = []
src_ctrl = unreal.IKRigController.get_controller(src_rig)
for c in src_ctrl.get_retarget_chains():
    src_chains.append(str(c.get_editor_property("chain_name")))

tgt_chains = []
tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)
for c in tgt_ctrl.get_retarget_chains():
    tgt_chains.append(str(c.get_editor_property("chain_name")))

out = {
    "chain_mappings": chains,
    "root_settings": root_settings,
    "src_chains": src_chains,
    "tgt_chains": tgt_chains
}

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rtg_deep_inspect.json", "w") as f:
    json.dump(out, f, indent=2)

print("RTG_DEEP_INSPECT_DONE")
