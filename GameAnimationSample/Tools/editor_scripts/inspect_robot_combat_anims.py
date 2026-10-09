import unreal, json

out = {}

# Check BP_RobotScout
bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
gen = bp.generated_class()
cdo = unreal.get_default_object(gen)
out["cdo_name"] = str(cdo)

# Check combat component on BP_RobotScout
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
    if o and "combat" in o.get_name().lower():
        out["combat_comp"] = o.get_name()

# Check DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis = da.get_editor_property("visuals")
out["da_visuals"] = {
    "attachment_socket": str(vis.get_editor_property("attachment_socket")),
    "holster_socket": str(vis.get_editor_property("holster_socket")),
    "hand_offset": str(vis.get_editor_property("hand_offset")),
    "holster_offset": str(vis.get_editor_property("holster_offset"))
}

# Check animations on DA_PlayerWeapon
anims = da.get_editor_property("animations")
out["da_anims"] = {
    "idle": str(anims.get_editor_property("idle_animation")) if hasattr(anims, 'idle_animation') else None,
    "equip": str(anims.get_editor_property("equip_animation")) if hasattr(anims, 'equip_animation') else None,
}

# Check all properties on anims struct
for prop in dir(anims):
    if not prop.startswith("_"):
        try:
            val = anims.get_editor_property(prop)
            if val:
                out["da_anims_" + prop] = str(val.get_path_name()) if hasattr(val, 'get_path_name') else str(val)
        except Exception:
            pass

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_robot_combat_anims.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ROBOT_COMBAT_ANIMS_DONE")
