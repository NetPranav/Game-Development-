import unreal, json

# Test exact rotations mapping
# Goal:
# Blade (0, 0, -1) -> thumb side of fist
# Edge (1, 0, 0) -> knuckle side of fist
# Flat (0, 1, 0) -> palm side of fist

rotations = {
    "pitch_180": unreal.Rotator(pitch=180.0, yaw=0.0, roll=0.0),
    "pitch_neg180": unreal.Rotator(pitch=-180.0, yaw=0.0, roll=0.0),
    "epic_composed": unreal.Rotator(pitch=180.0 - 3.176, yaw=-180.0, roll=0.0),
    "pitch_180_yaw_180": unreal.Rotator(pitch=180.0, yaw=180.0, roll=0.0),
    "pitch_90": unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
    "pitch_neg90": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
}

out = {}
for name, rot in rotations.items():
    q = rot.quaternion()
    blade = q.rotate_vector(unreal.Vector(0, 0, -1))
    edge = q.rotate_vector(unreal.Vector(1, 0, 0))
    flat = q.rotate_vector(unreal.Vector(0, 1, 0))
    
    out[name] = {
        "rot": [rot.pitch, rot.yaw, rot.roll],
        "blade_in_hand": [round(blade.x, 4), round(blade.y, 4), round(blade.z, 4)],
        "edge_in_hand": [round(edge.x, 4), round(edge.y, 4), round(edge.z, 4)],
        "flat_in_hand": [round(flat.x, 4), round(flat.y, 4), round(flat.z, 4)],
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/rotation_mapping_test.json", "w") as f:
    json.dump(out, f, indent=2)

print("ROTATION_MAPPING_TEST_DONE")
