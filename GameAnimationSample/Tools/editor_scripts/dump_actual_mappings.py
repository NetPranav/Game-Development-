import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

tgt_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
tgt_c = unreal.IKRigController.get_controller(tgt_rig)

mappings = {}
for c in tgt_c.get_retarget_chains():
    name = str(c.chain_name)
    mappings[name] = str(rc.get_source_chain(c.chain_name))

rs = rc.get_root_settings()
rs_props = {}
for prop in dir(rs):
    if not prop.startswith("_"):
        try:
            rs_props[prop] = str(getattr(rs, prop))
        except Exception:
            pass

out = {
    "mappings": mappings,
    "root_settings": rs_props
}

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rtg_actual_mappings.json", "w") as f:
    json.dump(out, f, indent=2)

print("MAPPINGS_DUMP_DONE")
