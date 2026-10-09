import unreal, json

out = {}
for path, name in [
    ("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A", "A"),
    ("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B", "B")
]:
    mesh = unreal.load_asset(path)
    socks = []
    # In UE5 Python:
    # We can check find_socket
    s = mesh.find_socket("Grip")
    if s:
        socks.append({
            "name": "Grip",
            "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
            "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
        })
    out[name] = socks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sword_grip_now.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_GRIP_NOW_DONE")
