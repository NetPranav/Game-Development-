import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

out = {}
pelvis_ctrl = rc.get_op_controller(0)
if pelvis_ctrl:
    out["src_pelvis"] = str(pelvis_ctrl.get_source_pelvis_bone())
    out["tgt_pelvis"] = str(pelvis_ctrl.get_target_pelvis_bone())
    s = pelvis_ctrl.get_settings()
    out["pelvis_settings"] = str(s)
    # Check properties of s
    if s:
        out["pelvis_settings_props"] = {p: str(s.get_editor_property(p)) for p in ["enable_pelvis_motion", "source_pelvis_bone", "target_pelvis_bone", "rotation_alpha", "translation_alpha"] if hasattr(s, p) or True}

root_ctrl = rc.get_op_controller(3)
if root_ctrl:
    s_root = root_ctrl.get_settings()
    out["root_settings"] = str(s_root)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_pelvis_settings.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_PELVIS_SETTINGS_DONE")
