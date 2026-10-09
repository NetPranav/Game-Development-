import unreal

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")

# Check if ABP has IKRetargeter variable
# In Unreal Editor Python, we can set default on the CDO or via Blueprint Editor Subsystem
cdo = unreal.get_default_object(abp.generated_class())
print("Old IKRetargeter on CDO:", cdo.get_editor_property("IKRetargeter"))

cdo.set_editor_property("IKRetargeter", rtg)
print("New IKRetargeter on CDO:", cdo.get_editor_property("IKRetargeter"))

# Compile and save
unreal.EditorAssetLibrary.save_loaded_asset(abp)
print("ABP Saved successfully!")
