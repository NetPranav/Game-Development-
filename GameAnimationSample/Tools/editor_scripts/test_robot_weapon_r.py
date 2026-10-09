import unreal, json

robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
sock = robot_mesh.find_socket("weapon_r")

out = {
    "has_weapon_r": sock is not None,
}
if sock:
    out["bone_name"] = str(sock.get_editor_property("bone_name"))
    loc = sock.get_editor_property("relative_location")
    rot = sock.get_editor_property("relative_rotation")
    out["loc"] = [loc.x, loc.y, loc.z]
    out["rot"] = [rot.pitch, rot.yaw, rot.roll]

with open(unreal.Paths.project_saved_dir() + "RRTools/test_robot_weapon_r.json", "w") as f:
    json.dump(out, f, indent=2)

print("ROBOT_WEAPON_R_DONE")
