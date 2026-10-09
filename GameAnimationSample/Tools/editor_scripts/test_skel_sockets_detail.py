import unreal, json

robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot_mesh.get_editor_property("skeleton")

out = {
    "skel": str(skel.get_path_name()) if skel else None,
    "skel_sockets": []
}

for s in skel.get_editor_property("sockets"):
    out["skel_sockets"].append({
        "name": str(s.get_editor_property("socket_name")),
        "bone": str(s.get_editor_property("bone_name")),
        "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
        "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
    })

# If we create an actor with SkeletalMeshComponent:
# does does_socket_exist("weapon_r") return true?
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_skel_sockets_detail.json", "w") as f:
    json.dump(out, f, indent=2)

print("SKEL_SOCKETS_DONE")
