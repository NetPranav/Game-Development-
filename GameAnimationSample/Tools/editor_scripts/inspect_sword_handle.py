import unreal, json

sword = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
bounds = sword.get_bounds()

# Check sockets
sockets = {}
for s in ["Grip", "TraceStart", "TraceEnd"]:
    sock = sword.find_socket(s)
    if sock:
        sockets[s] = [sock.relative_location.x, sock.relative_location.y, sock.relative_location.z]

out = {
    "bounds_origin": [bounds.origin.x, bounds.origin.y, bounds.origin.z],
    "bounds_extent": [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z],
    "sockets": sockets
}

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sword_handle.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SWORD_HANDLE_DONE")
