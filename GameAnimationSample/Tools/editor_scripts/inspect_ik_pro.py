import unreal, json

out = {}

rig = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/IK_ProSwordShield")
if rig:
    rc = unreal.IKRigController.get_controller(rig)
    out["chains"] = []
    for c in rc.get_retarget_chains():
        d = c.to_dict()
        out["chains"].append(d)
    out["retarget_root"] = str(rc.get_retarget_root())

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
if rtg:
    rtc = unreal.IKRetargeterController.get_controller(rtg)
    # Check chain mappings
    tgt_rig = rtc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
    tgt_rc = unreal.IKRigController.get_controller(tgt_rig)
    mappings = {}
    for c in tgt_rc.get_retarget_chains():
        name = str(c.to_dict()["chain_name"])
        src_c = str(rtc.get_source_chain(name))
        mappings[name] = src_c
    out["mappings"] = mappings

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ik_pro.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_IK_PRO_DONE")
