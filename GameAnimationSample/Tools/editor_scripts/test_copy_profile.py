import unreal, json

rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
prof = unreal.RetargetProfileLibrary.copy_retarget_profile_from_retarget_asset(rtg)

res = unreal.RetargetProfileLibrary.get_op_controller_from_retarget_profile(prof, "Retarget IK Goals")
print(f"res type: {type(res)}, val: {res}")

ctrl = res[0] if isinstance(res, tuple) else res
print(f"ctrl: {ctrl}")
cast_ctrl = unreal.IKRetargetIKChainsController.cast(ctrl) if ctrl else None
print(f"cast_ctrl: {cast_ctrl}")

out = {
    "res_type": str(type(res)),
    "ctrl_str": str(ctrl),
    "cast_success": cast_ctrl is not None,
    "class": ctrl.get_class().get_name() if ctrl else None
}

with open(unreal.Paths.project_saved_dir() + "RRTools/test_copy_profile.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_COPY_PROFILE_SUCCESS")
