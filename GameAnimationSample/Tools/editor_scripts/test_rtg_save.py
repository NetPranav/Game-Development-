import unreal

eal = unreal.EditorAssetLibrary

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

print("Before set Root:", rc.get_source_chain("Root"))
rc.set_source_chain("Root", unreal.Name(""))
print("After set empty Name:", rc.get_source_chain("Root"))

rc.set_source_chain("Root", unreal.Name("None"))
print("After set None Name:", rc.get_source_chain("Root"))

rc.set_source_chain("Root", "")
print("After set empty str:", rc.get_source_chain("Root"))

eal.save_loaded_asset(rtg)
eal.save_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")

# Reload and check
rtg2 = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc2 = unreal.IKRetargeterController.get_controller(rtg2)
print("After reload Root:", rc2.get_source_chain("Root"))
