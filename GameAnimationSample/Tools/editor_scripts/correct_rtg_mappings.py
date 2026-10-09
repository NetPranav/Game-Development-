import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

results = {}
results["before"] = str(rc.get_source_chain("Root"))

# Now call correctly: source_chain_name, target_chain_name
# To clear source for target "Root": source_chain_name = "", target_chain_name = "Root"
ret = rc.set_source_chain("", "Root")
results["ret_empty"] = ret
results["after_empty"] = str(rc.get_source_chain("Root"))

if not results["after_empty"] or results["after_empty"] == "None":
    print("SUCCESS: Root unmapped!")
else:
    # try "None"
    ret2 = rc.set_source_chain("None", "Root")
    results["ret_None"] = ret2
    results["after_None"] = str(rc.get_source_chain("Root"))

# Also unmap RightToe and LeftToe, and Metacarpals
for extra_target in ["RightToe", "LeftToe", 
    "LeftIndexMetacarpal", "LeftMiddleMetacarpal", "LeftRingMetacarpal", "LeftPinkyMetacarpal", 
    "RightIndexMetacarpal", "RightMiddleMetacarpal", "RightRingMetacarpal", "RightPinkyMetacarpal"]:
    rc.set_source_chain("", extra_target)

# Correctly map the real chains: (source, target)
valid_maps = [
    ("Spine", "Spine"),
    ("Neck", "Neck"),
    ("Head", "Head"),
    ("LeftClavicle", "LeftClavicle"),
    ("LeftArm", "LeftArm"),
    ("LeftLeg", "LeftLeg"),
    ("RightClavicle", "RightClavicle"),
    ("RightArm", "RightArm"),
    ("RightLeg", "RightLeg"),
    ("LeftThumb", "LeftThumb"),
    ("LeftIndex", "LeftIndex"),
    ("LeftMiddle", "LeftMiddle"),
    ("LeftRing", "LeftRing"),
    ("LeftPinky", "LeftPinky"),
    ("RightThumb", "RightThumb"),
    ("RightIndex", "RightIndex"),
    ("RightMiddle", "RightMiddle"),
    ("RightRing", "RightRing"),
    ("RightPinky", "RightPinky")
]

for src, tgt in valid_maps:
    rc.set_source_chain(src, tgt)

# Dump all mappings now
tgt_rig = rc.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)
tgt_c = unreal.IKRigController.get_controller(tgt_rig)
final_mappings = {}
for c in tgt_c.get_retarget_chains():
    name = str(c.chain_name)
    final_mappings[name] = str(rc.get_source_chain(c.chain_name))

results["final_mappings"] = final_mappings

eal = unreal.EditorAssetLibrary
eal.save_loaded_asset(rtg)

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/verified_rtg_mappings.json", "w") as f:
    json.dump(results, f, indent=2)

print("VERIFIED_MAPPINGS_DONE")
