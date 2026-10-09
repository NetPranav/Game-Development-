import unreal, json

out = {}

sword = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
sock_list = []
for name in ["Grip", "TraceStart", "TraceEnd", "hand_r"]:
    try:
        s = sword.find_socket(name)
        if s:
            sock_list.append({
                "name": name,
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            })
    except Exception as e:
        pass
out["sword_sockets_found"] = sock_list

# Check DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis = da.get_editor_property("visuals")
t = vis.get_editor_property("hand_offset")
out["da_hand_offset"] = {
    "loc": [t.translation.x, t.translation.y, t.translation.z],
    "rot": [t.rotation.rotator().pitch, t.rotation.rotator().yaw, t.rotation.rotator().roll],
    "scale": [t.scale3d.x, t.scale3d.y, t.scale3d.z]
}

# Check robot mesh reference pose for hand_r
robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")
out["robot_skeleton"] = skel.get_name()

# Find sockets on skeletal mesh
skel_socks = []
for name in ["hand_r", "hand_rSocket", "hand_l", "spine_03", "weapon_r", "Grip"]:
    try:
        s = robot.find_socket(name)
        if s:
            skel_socks.append({
                "name": name,
                "bone": str(s.get_editor_property("bone_name")),
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            })
    except Exception as e:
        pass
out["robot_mesh_sockets_found"] = skel_socks

# Also check SKM_UE4_Mannequin sockets
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")
man_socks = []
if mannequin:
    for name in ["hand_r", "hand_rSocket", "hand_l", "spine_03", "weapon_r", "Grip"]:
        try:
            s = mannequin.find_socket(name)
            if s:
                man_socks.append({
                    "name": name,
                    "bone": str(s.get_editor_property("bone_name")),
                    "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                    "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
                })
        except Exception:
            pass
out["mannequin_sockets_found"] = man_socks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_grip_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_GRIP_DETAILS_SUCCESS")
