import unreal

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

print("Before:", rc.get_source_chain("Root"))

for try_val in ["None", "", unreal.Name("None"), unreal.Name(""), None]:
    try:
        rc.set_source_chain("Root", try_val)
        print(f"Tried {try_val}: result = {rc.get_source_chain('Root')}")
    except Exception as e:
        print(f"Tried {try_val}: exception {e}")

eal = unreal.EditorAssetLibrary
eal.save_loaded_asset(rtg)
