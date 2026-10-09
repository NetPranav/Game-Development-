import unreal, json

robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot.get_editor_property("skeleton")

out = {
    "robot_dir": [m for m in dir(robot) if "sock" in m.lower()],
    "skel_dir": [m for m in dir(skel) if "sock" in m.lower()],
}

# Can we create SkeletalMeshSocket and add to robot?
try:
    # Check if robot has append_socket or similar
    out["robot_methods"] = [m for m in dir(robot) if any(k in m.lower() for k in ["add", "socket", "attach"])]
except Exception as e:
    out["robot_err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/test_skel_socket_out.json", "w") as f:
    json.dump(out, f, indent=2)

print("SAVED_OUTPUT")
