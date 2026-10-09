import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")

out = {}

# Check bone transform of hand_r in ref skeleton
try:
    ref_skel = robot.get_editor_property("ref_skeleton")
    out["ref_skel"] = str(ref_skel)
except Exception as e:
    out["ref_skel_err"] = str(e)

# Let's inspect the socket weapon_r on SKM_UE4_Mannequin
mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")
ws = mannequin.find_socket("weapon_r")
if ws:
    out["weapon_r"] = {
        "loc": [ws.relative_location.x, ws.relative_location.y, ws.relative_location.z],
        "rot": [ws.relative_rotation.pitch, ws.relative_rotation.yaw, ws.relative_rotation.roll]
    }

# Also let's check what socket Greystone uses for his sword:
gs = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Meshes/Greystone")
if gs:
    gs_socks = []
    for s_name in ["weapon_r", "hand_r", "FX_Sword_Base", "FX_Sword_Tip", "Grip", "Sword"]:
        s = gs.find_socket(s_name)
        if s:
            gs_socks.append({
                "name": s_name,
                "bone": str(s.get_editor_property("bone_name")),
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            })
    out["greystone_sockets"] = gs_socks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_weapon_socket_ref.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_WEAPON_SOCKET_REF_DONE")
