import unreal, json

mesh = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")

# Let's inspect bounds and sockets
bounds = mesh.get_bounds()
out = {
    "origin": [bounds.origin.x, bounds.origin.y, bounds.origin.z],
    "box_extent": [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z],
    "sphere_radius": bounds.sphere_radius,
}

# In inspect_sword_measurements.json:
# box_extent was: X=18.68, Y=7.57, Z=76.14
# So Z is length (total height ~152 cm)
# X is width (extent 18.68 -> total width 37.3 cm) -> Crossguard!
# Y is thickness (extent 7.57 -> total thickness 15.1 cm) -> Flat / depth!

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sword_axes_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SWORD_AXES_DETAILS_DONE")
