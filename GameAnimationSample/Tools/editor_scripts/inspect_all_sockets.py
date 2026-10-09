import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")

out = {}

# Check sockets on mannequin
ms = mannequin.find_socket("weapon_r")
if ms:
    out["mannequin_weapon_r"] = {
        "bone": str(ms.get_editor_property("bone_name")),
        "loc": [ms.relative_location.x, ms.relative_location.y, ms.relative_location.z],
        "rot": [ms.relative_rotation.pitch, ms.relative_rotation.yaw, ms.relative_rotation.roll]
    }

# Check if UE5 mannequin (SKM_Manny / Quinn) also has weapon sockets
for manny_path in ["/Game/Characters/Mannequins/Meshes/SKM_Manny", "/Game/Characters/Mannequins/Meshes/SKM_Quinn"]:
    m = unreal.load_asset(manny_path)
    if m:
        for sname in ["weapon_r", "hand_rSocket", "Grip"]:
            s = m.find_socket(sname)
            if s:
                out[manny_path + "_" + sname] = {
                    "bone": str(s.get_editor_property("bone_name")),
                    "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                    "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
                }

# Check sockets on robot's skeleton
skel = robot.get_editor_property("skeleton")
# Can we get sockets via python?
try:
    # unreal.Skeleton doesn't expose sockets property directly in 5.8 sometimes, but let's check
    for sname in ["weapon_r", "hand_r", "hand_rSocket"]:
        s = robot.find_socket(sname)
        if s:
            out["robot_" + sname] = {
                "bone": str(s.get_editor_property("bone_name")),
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            }
except Exception as e:
    out["robot_err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ue_sockets.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_UE_SOCKETS_DONE")
