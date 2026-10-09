import unreal, json

sword_a = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
sword_b = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B")

out = {}

for name, mesh in [("SM_Sword_A", sword_a), ("SM_Sword_B", sword_b)]:
    if not mesh:
        continue
    b = mesh.get_bounds()
    sockets = []
    # In UE5 Python, find_socket works on static mesh
    for s_name in ["Grip", "TraceStart", "TraceEnd"]:
        s = mesh.find_socket(s_name)
        if s:
            sockets.append({
                "name": s_name,
                "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
                "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
            })
    out[name] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "box_extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
        "sockets": sockets
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sword_measurements.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SWORD_MEASUREMENTS_DONE")
