import unreal

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)

print(f"Num ops before: {rc.get_num_retarget_ops()}")
idx = rc.add_retarget_op("/Script/IKRig.IKRetargetIKChainsOp")
print(f"Added op at index: {idx}")
if idx >= 0:
    renamed = rc.set_op_name("Retarget IK Goals", idx)
    print(f"Renamed op to: {renamed}")
    ctrl = rc.get_op_controller(idx)
    print(f"Controller: {ctrl}, class: {ctrl.get_class().get_name() if ctrl else None}")

    # Check if GetOpControllerFromRetargetProfile works
    # Save the RTG asset
    saved = unreal.EditorAssetLibrary.save_loaded_asset(rtg)
    print(f"Saved RTG: {saved}")
