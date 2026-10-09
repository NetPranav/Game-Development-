import unreal, json

eal = unreal.EditorAssetLibrary
out = {}

for path, name in [
    ("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A", "A"),
    ("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B", "B")
]:
    mesh = unreal.load_asset(path)
    s = mesh.find_socket("Grip")
    if s:
        s.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 11.0))
        s.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
        eal.save_loaded_asset(mesh)
        out[name] = "Updated Grip to (0,0,11)"
    else:
        sock = unreal.StaticMeshSocket(outer=mesh)
        sock.set_editor_property("socket_name", "Grip")
        sock.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 11.0))
        sock.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
        mesh.add_socket(sock)
        eal.save_loaded_asset(mesh)
        out[name] = "Added Grip at (0,0,11)"

with open(unreal.Paths.project_saved_dir() + "RRTools/update_sword_grip.json", "w") as f:
    json.dump(out, f, indent=2)

print("UPDATE_SWORD_GRIP_DONE")
