import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}

# Check help on set_source_chain
out["doc"] = rc.set_source_chain.__doc__

# Check rtg properties
props = []
for p in dir(rtg):
    if not p.startswith("_"):
        props.append(p)
out["rtg_props"] = props

# Check retarget asset editor properties
for prop_name in ["retarget_profile", "chain_mapping", "source_ik_rig", "target_ik_rig"]:
    try:
        val = rtg.get_editor_property(prop_name)
        out[f"rtg_{prop_name}"] = str(val)
    except Exception as e:
        out[f"rtg_{prop_name}_err"] = str(e)

# Check all chain settings in rc
try:
    all_settings = rc.get_all_chain_settings()
    out["num_chain_settings"] = len(all_settings)
    out["chain_settings_sample"] = [str(x) for x in all_settings[:3]]
except Exception as e:
    out["all_chain_settings_err"] = str(e)

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rc_methods_help.json", "w") as f:
    json.dump(out, f, indent=2)

print("HELP_DUMP_DONE")
