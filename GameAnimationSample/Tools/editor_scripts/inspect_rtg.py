import unreal
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)
for i in range(rc.get_num_retarget_ops()):
    print("OP", i, rc.get_op_name(i), rc.get_retarget_op_enabled(i))
try:
    rs = rc.get_root_settings()
    print("ROOT", rs.to_dict() if hasattr(rs, "to_dict") else rs)
except Exception as e:
    print("root err", e)
try:
    pm = rc.get_op_controller(rc.get_index_of_op_by_name("Pelvis Motion"))
    print("PELVIS", pm)
except Exception as e:
    print("pelvis err", e)
