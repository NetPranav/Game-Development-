import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
out = {}

# Let's inspect the sockets on robot
sk_socks = []
for s in robot.get_editor_property("mesh_clothing_assets"):
    pass

# We can query find_socket for standard socket names
for name in ["hand_r", "hand_l", "weapon_r", "weapon_l", "Grip", "spine_03"]:
    s = robot.find_socket(name)
    if s:
        sk_socks.append({
            "name": name,
            "bone": str(s.get_editor_property("bone_name")),
            "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
            "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
        })
out["robot_mesh_sockets"] = sk_socks

# Let's check the skeleton asset
skel = robot.get_editor_property("skeleton")
out["skeleton_path"] = skel.get_path_name()

# Let's check SKM_UE4_Mannequin sockets
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")
msocks = []
for name in ["hand_r", "hand_l", "weapon_r", "weapon_l", "Grip", "spine_03"]:
    s = mannequin.find_socket(name)
    if s:
        msocks.append({
            "name": name,
            "bone": str(s.get_editor_property("bone_name")),
            "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
            "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
        })
out["mannequin_sockets"] = msocks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sk_robot_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SK_ROBOT_DETAILS_DONE")
