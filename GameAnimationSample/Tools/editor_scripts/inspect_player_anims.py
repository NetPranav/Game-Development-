import unreal, json

player_bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
player_cdo = unreal.get_default_object(player_bp.generated_class())

out = {}
# Check anim class on player mesh
mesh = player_cdo.get_editor_property("mesh")
if mesh:
    anim_class = mesh.get_editor_property("anim_class")
    out["player_anim_class"] = str(anim_class.get_path_name()) if anim_class else None

# Check weapon DA
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
out["da_visuals"] = {
    "mesh": str(da.visuals.static_mesh.get_path_name()) if da.visuals.static_mesh else None,
    "attachment_socket": str(da.visuals.attachment_socket),
    "hand_offset": {
        "loc": [da.visuals.hand_offset.translation.x, da.visuals.hand_offset.translation.y, da.visuals.hand_offset.translation.z],
        "rot": [da.visuals.hand_offset.rotation.rotator().pitch, da.visuals.hand_offset.rotation.rotator().yaw, da.visuals.hand_offset.rotation.rotator().roll]
    }
}

# Check UAL1/UAL2 anim sequences
ual_seq = unreal.load_asset("/Game/RR/Player/AM_P_UAL_L1")
out["ual1"] = str(ual_seq)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_player_anims.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_PLAYER_ANIMS_DONE")
