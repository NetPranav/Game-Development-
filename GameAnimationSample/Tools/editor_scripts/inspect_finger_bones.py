import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")

# Let's inspect bone transforms relative to hand_r in ref pose
# We can use SKM_UE4_Mannequin or SK_Robot_scout_R21
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")

out = {}

# Sockets on mannequin
s_wep = mannequin.find_socket("weapon_r")
if s_wep:
    out["weapon_r_socket"] = {
        "bone": str(s_wep.get_editor_property("bone_name")),
        "loc": [s_wep.relative_location.x, s_wep.relative_location.y, s_wep.relative_location.z],
        "rot": [s_wep.relative_rotation.pitch, s_wep.relative_rotation.yaw, s_wep.relative_rotation.roll]
    }

# Also let's check Greystone:
# Greystone uses sword_bottom and sword_top bone
# What about other weapon setups?
# Let's inspect Action RPG or any animation sample if present in project
ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/Engine", "StaticMeshSocket"))
out["static_mesh_sockets_count"] = len(assets)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_finger_bones.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_FINGER_BONES_DONE")
