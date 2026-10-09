import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")

out = {}

# Check if there is weapon_r socket on UE4_Mannequin_Skeleton in the project
# Let's inspect all sockets on UE4_Mannequin_Skeleton
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")
# In inspect_grip_details.json, we saw mannequin had socket weapon_r on bone hand_r:
# loc: [-10.764, 5.811, -0.728]
# rot: [-3.176, -180.0, 0.0]

out["mannequin_weapon_r_socket"] = {
    "loc": [-10.764, 5.811, -0.728],
    "rot": [-3.176, -180.0, 0.0]
}

# Let's check what sockets exist on SK_Mannequin (UE5 / GASP mannequin)
ue5_mesh = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny")
if not ue5_mesh:
    ue5_mesh = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SK_Mannequin")
if ue5_mesh:
    ue5_socks = []
    for s_name in ["weapon_r", "hand_rSocket", "hand_r", "Grip"]:
        s = ue5_mesh.find_socket(s_name)
        if s:
            ue5_socks.append({
                "name": s_name,
                "bone": str(s.get_editor_property("bone_name")),
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            })
    out["ue5_mesh_sockets"] = ue5_socks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ue5_sockets.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_UE5_SOCKETS_DONE")
