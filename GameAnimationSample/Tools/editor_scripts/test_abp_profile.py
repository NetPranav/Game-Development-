import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
cdo = unreal.get_default_object(abp.generated_class())
rtg = cdo.get_editor_property("IKRetargeter")

out = {
    "rtg": str(rtg.get_path_name()) if rtg else None
}

if rtg:
    prof = unreal.RetargetProfileLibrary.create_retarget_profile_from_retargeter(rtg)
    out["has_prof"] = prof is not None
    ctrl = unreal.RetargetProfileLibrary.get_op_controller_from_retarget_profile(prof, "Retarget IK Goals")
    out["ctrl"] = str(ctrl)
    if ctrl:
        out["ctrl_class"] = ctrl.get_class().get_name()
        chains_ctrl = unreal.IKRetargetIKChainsController.cast(ctrl)
        out["cast_success"] = chains_ctrl is not None

    # Let's also set the CDO's RetargetProfile so its default profile has this op controller!
    cdo.set_editor_property("RetargetProfile", prof)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp=abp)
    unreal.EditorAssetLibrary.save_loaded_asset(abp)
    out["saved"] = True

with open(unreal.Paths.project_saved_dir() + "RRTools/test_abp_profile.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_ABP_PROFILE_DONE")
