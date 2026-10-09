import unreal, json

eal = unreal.EditorAssetLibrary

# 1. Update SM_Sword_A and SM_Sword_B sockets
for mesh_path in [
    "/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A",
    "/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B"
]:
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        continue
    
    # Grip: Center of leather handle (Z = 130)
    # TraceStart: Base of blade at skull crossguard (Z = 108)
    # TraceEnd: Tip of blade (Z = 2)
    socket_defs = {
        "Grip": (unreal.Vector(0.0, 0.0, 130.0), unreal.Rotator(0.0, 0.0, 0.0)),
        "TraceStart": (unreal.Vector(0.0, 0.0, 108.0), unreal.Rotator(0.0, 0.0, 0.0)),
        "TraceEnd": (unreal.Vector(0.0, 0.0, 2.0), unreal.Rotator(0.0, 0.0, 0.0))
    }
    
    for s_name, (s_loc, s_rot) in socket_defs.items():
        s = mesh.find_socket(s_name)
        if s:
            s.set_editor_property("relative_location", s_loc)
            s.set_editor_property("relative_rotation", s_rot)
        else:
            sock = unreal.StaticMeshSocket(outer=mesh)
            sock.set_editor_property("socket_name", s_name)
            sock.set_editor_property("relative_location", s_loc)
            sock.set_editor_property("relative_rotation", s_rot)
            mesh.add_socket(sock)
            
    eal.save_loaded_asset(mesh)
    print(f"Updated sockets on {mesh_path}")

# 2. Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis = da.get_editor_property("visuals")

# Position: center of palm relative to hand_r wrist joint
loc = unreal.Vector(-10.764, 5.811, -0.728)
# Rotation: Pitch 180 degrees (inverts -Z blade to +Z in hand, +X edge to -X in knuckles)
rot = unreal.Rotator(pitch=180.0, yaw=0.0, roll=0.0)
t = unreal.Transform(loc, rot, unreal.Vector(1, 1, 1))

vis.set_editor_property("attachment_socket", "hand_r")
vis.set_editor_property("hand_offset", t)
da.set_editor_property("visuals", vis)

eal.save_loaded_asset(da)
print("Updated DA_PlayerWeapon")

# 3. Verify
da_check = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis_check = da_check.get_editor_property("visuals")
ho_check = vis_check.get_editor_property("hand_offset")
r_check = ho_check.rotation.rotator()

out = {
    "attachment_socket": str(vis_check.get_editor_property("attachment_socket")),
    "loc": [ho_check.translation.x, ho_check.translation.y, ho_check.translation.z],
    "rot": [r_check.pitch, r_check.yaw, r_check.roll]
}

with open(unreal.Paths.project_saved_dir() + "RRTools/final_weapon_setup.json", "w") as f:
    json.dump(out, f, indent=2)

print("FINAL_WEAPON_SETUP_DONE")
