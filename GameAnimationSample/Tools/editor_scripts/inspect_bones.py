import unreal, json

out = {}

# Test Rotator args
r_pos = unreal.Rotator(10, 20, 30)
r_kw = unreal.Rotator(pitch=10, yaw=20, roll=30)
out["rot_test"] = {
    "pos": [r_pos.pitch, r_pos.yaw, r_pos.roll],
    "kw": [r_kw.pitch, r_kw.yaw, r_kw.roll]
}

# Inspect SK_Robot_scout_R21 skeleton & bones
robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
skel = robot_mesh.get_editor_property("skeleton")
ref_skel = robot_mesh.get_editor_property("ref_skeleton") if hasattr(robot_mesh, 'ref_skeleton') else None

# Check sockets on robot mesh or skeleton
try:
    num_sockets = unreal.SkeletonLibrary.get_num_sockets(skel) if hasattr(unreal, 'SkeletonLibrary') else -1
    out["num_sockets"] = num_sockets
except Exception as e:
    out["skel_lib_err"] = str(e)

# Check all sockets on the skeleton
sockets = []
for s in skel.get_editor_property("sockets"):
    sockets.append({
        "socket_name": str(s.get_editor_property("socket_name")),
        "bone_name": str(s.get_editor_property("bone_name")),
        "rel_loc": str(s.get_editor_property("relative_location")),
        "rel_rot": str(s.get_editor_property("relative_rotation")),
    })
out["skeleton_sockets"] = sockets

# Let's check sockets on the skeletal mesh itself
mesh_sockets = []
try:
    for s in robot_mesh.get_editor_property("mesh_sockets"):
        mesh_sockets.append({
            "socket_name": str(s.get_editor_property("socket_name")),
            "bone_name": str(s.get_editor_property("bone_name")),
            "rel_loc": str(s.get_editor_property("relative_location")),
            "rel_rot": str(s.get_editor_property("relative_rotation")),
        })
except Exception as e:
    out["mesh_sockets_err"] = str(e)
out["mesh_sockets"] = mesh_sockets

# Check what sockets exist on the mannequin skeleton: UE4_Mannequin_Skeleton vs SK_Mannequin
mannequin_skel = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/UE4_Mannequin_Skeleton")
if mannequin_skel:
    m_sockets = []
    for s in mannequin_skel.get_editor_property("sockets"):
        m_sockets.append({
            "socket_name": str(s.get_editor_property("socket_name")),
            "bone_name": str(s.get_editor_property("bone_name")),
            "rel_loc": str(s.get_editor_property("relative_location")),
            "rel_rot": str(s.get_editor_property("relative_rotation")),
        })
    out["mannequin_sockets"] = m_sockets

# Also check UEFN mannequin / GASP mannequin skeleton
gasp_skel = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SK_Mannequin")
if gasp_skel:
    out["gasp_skel"] = str(gasp_skel.get_editor_property("skeleton"))

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_bones.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BONES_DONE")
