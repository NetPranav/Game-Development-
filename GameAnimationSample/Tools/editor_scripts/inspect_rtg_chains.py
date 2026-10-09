import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}

tgt_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
src_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.SOURCE)

# In target rig (IK_UEFN_Mannequin), get all retarget chains
tgt_chains = []
if tgt_rig:
    rig_c = unreal.IKRigController.get_controller(tgt_rig)
    for c in rig_c.get_retarget_chains():
        c_name = c.chain_name
        src_c = rc.get_source_chain(c_name)
        tgt_chains.append({
            "chain_name": str(c_name),
            "start_bone": str(c.start_bone.bone_name),
            "end_bone": str(c.end_bone.bone_name),
            "source_mapped": str(src_c)
        })

out["tgt_chains"] = tgt_chains

# In source rig (IK_ProSwordShield), get all chains
src_chains = []
if src_rig:
    rig_s = unreal.IKRigController.get_controller(src_rig)
    for c in rig_s.get_retarget_chains():
        src_chains.append({
            "chain_name": str(c.chain_name),
            "start_bone": str(c.start_bone.bone_name),
            "end_bone": str(c.end_bone.bone_name)
        })
out["src_chains"] = src_chains

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_rtg_chains.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_RTG_CHAINS_DONE")
