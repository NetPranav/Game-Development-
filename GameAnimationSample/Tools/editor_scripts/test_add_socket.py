import unreal, json

# We can spawn or inspect BP_RobotScout and test socket attachment
out = {}

# Test adding socket weapon_r to SK_Robot_scout_R21
robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
eal = unreal.EditorAssetLibrary

existing_socket = robot.find_socket("weapon_r")
out["had_existing_weapon_r"] = bool(existing_socket)

if not existing_socket:
    sock = unreal.SkeletalMeshSocket(outer=robot)
    sock.set_editor_property("socket_name", "weapon_r")
    sock.set_editor_property("bone_name", "hand_r")
    sock.set_editor_property("relative_location", unreal.Vector(-10.764, 5.811, -0.728))
    sock.set_editor_property("relative_rotation", unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0))
    robot.add_socket(sock)
    eal.save_loaded_asset(robot)
    out["added_weapon_r"] = True
else:
    out["added_weapon_r"] = False

# Verify socket on robot
s = robot.find_socket("weapon_r")
if s:
    out["robot_weapon_r"] = {
        "bone": str(s.get_editor_property("bone_name")),
        "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
        "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
    }

# Also test on UE4_Mannequin_Skeleton if possible
skel = robot.get_editor_property("skeleton")
out["skel_path"] = skel.get_path_name()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_add_socket.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_ADD_SOCKET_DONE")
