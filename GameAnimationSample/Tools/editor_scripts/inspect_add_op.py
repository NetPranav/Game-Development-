import unreal

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)

with open(unreal.Paths.project_saved_dir() + "RRTools/add_op_doc.txt", "w") as f:
    f.write(str(rc.add_retarget_op.__doc__))

print("DOC_SAVED")
