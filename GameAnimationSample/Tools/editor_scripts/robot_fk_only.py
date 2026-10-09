import unreal
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)
for name in ("Offset Goals", "Speed Plant IK Goals", "Run IK Rig", "Pole Vector Alignment"):
    i = rc.get_index_of_op_by_name(name)
    rc.set_retarget_op_enabled(i, False)
    print("DISABLED", name, i)
unreal.EditorAssetLibrary.save_loaded_asset(rtg)
print("FK_DONE")
