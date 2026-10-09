import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")

out = {}

mannequin = unreal.load_asset("/Game/Characters/UE4_Mannequin/Meshes/SKM_UE4_Mannequin")
ms = mannequin.find_socket("weapon_r")
if ms:
    r = ms.relative_rotation
    t_sock_rel = unreal.Transform(
        ms.relative_location,
        ms.relative_rotation,
        unreal.Vector(1, 1, 1)
    )
    out["sock_rel_loc"] = [ms.relative_location.x, ms.relative_location.y, ms.relative_location.z]
    out["sock_rel_rot"] = [r.pitch, r.yaw, r.roll]
    
    fwd = r.get_forward_vector()
    right = r.get_right_vector()
    up = r.get_up_vector()
    out["sock_in_bone_axes"] = {
        "forward_X": [fwd.x, fwd.y, fwd.z],
        "right_Y": [right.x, right.y, right.z],
        "up_Z": [up.x, up.y, up.z]
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_weapon_axes.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_WEAPON_AXES_DONE")
