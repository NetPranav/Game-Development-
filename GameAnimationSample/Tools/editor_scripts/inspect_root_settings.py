import unreal, json

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)
rs = rc.get_root_settings()

out = {
    "source_chain_root": str(rc.get_source_chain("Root")),
    "root_settings": str(rs),
    "root_settings_dict": {
        "retain_root_height": rs.get_editor_property("retain_root_height") if hasattr(rs, "retain_root_height") else "N/A"
    } if rs else {}
}

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rtg_root_settings.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ROOT_SETTINGS_DONE")
