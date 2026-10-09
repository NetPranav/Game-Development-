import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

results = {}
results["initial"] = str(rc.get_source_chain("Root"))

test_values = [
    ("str_None", "None"),
    ("str_empty", ""),
    ("name_None", unreal.Name("None")),
    ("name_empty", unreal.Name("")),
    ("none_val", None)
]

for label, val in test_values:
    try:
        rc.set_source_chain("Root", val)
        results[label] = str(rc.get_source_chain("Root"))
    except Exception as e:
        results[label] = f"error: {e}"

# Also check if we can remove or set chain
eal = unreal.EditorAssetLibrary
eal.save_loaded_asset(rtg)

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/test_clear_results.json", "w") as f:
    json.dump(results, f, indent=2)

print("TEST_CLEAR_DONE")
