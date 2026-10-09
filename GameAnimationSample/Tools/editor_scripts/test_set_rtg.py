import unreal

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")

cdo = unreal.get_default_object(abp.generated_class())
print("BEFORE:", cdo.get_editor_property("IKRetargeter"))
try:
    cdo.set_editor_property("IKRetargeter", rtg)
    print("SET SUCCESS:", cdo.get_editor_property("IKRetargeter"))
except Exception as e:
    print("SET ERROR:", e)
