import unreal, json

r = unreal.Rotator(pitch=0.0, yaw=0.0, roll=-90.0)
q = r.quaternion()

# Weapon vectors:
# Blade: (0, 0, -1)
# Pommel: (0, 0, 1)
# Crossguard: (1, 0, 0)
# Flat: (0, 1, 0)

out = {
    "blade_dir_in_hand": [q.rotate_vector(unreal.Vector(0, 0, -1)).x, q.rotate_vector(unreal.Vector(0, 0, -1)).y, q.rotate_vector(unreal.Vector(0, 0, -1)).z],
    "pommel_dir_in_hand": [q.rotate_vector(unreal.Vector(0, 0, 1)).x, q.rotate_vector(unreal.Vector(0, 0, 1)).y, q.rotate_vector(unreal.Vector(0, 0, 1)).z],
    "crossguard_dir_in_hand": [q.rotate_vector(unreal.Vector(1, 0, 0)).x, q.rotate_vector(unreal.Vector(1, 0, 0)).y, q.rotate_vector(unreal.Vector(1, 0, 0)).z],
    "flat_dir_in_hand": [q.rotate_vector(unreal.Vector(0, 1, 0)).x, q.rotate_vector(unreal.Vector(0, 1, 0)).y, q.rotate_vector(unreal.Vector(0, 1, 0)).z]
}

# Compare with roll = 90
r2 = unreal.Rotator(pitch=0.0, yaw=0.0, roll=90.0)
q2 = r2.quaternion()
out["roll_pos90"] = {
    "blade_dir_in_hand": [q2.rotate_vector(unreal.Vector(0, 0, -1)).x, q2.rotate_vector(unreal.Vector(0, 0, -1)).y, q2.rotate_vector(unreal.Vector(0, 0, -1)).z],
    "pommel_dir_in_hand": [q2.rotate_vector(unreal.Vector(0, 0, 1)).x, q2.rotate_vector(unreal.Vector(0, 0, 1)).y, q2.rotate_vector(unreal.Vector(0, 0, 1)).z],
    "crossguard_dir_in_hand": [q2.rotate_vector(unreal.Vector(1, 0, 0)).x, q2.rotate_vector(unreal.Vector(1, 0, 0)).y, q2.rotate_vector(unreal.Vector(1, 0, 0)).z],
    "flat_dir_in_hand": [q2.rotate_vector(unreal.Vector(0, 1, 0)).x, q2.rotate_vector(unreal.Vector(0, 1, 0)).y, q2.rotate_vector(unreal.Vector(0, 1, 0)).z]
}

# Compare with pitch = 90
r3 = unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0)
q3 = r3.quaternion()
out["pitch_pos90"] = {
    "blade_dir_in_hand": [q3.rotate_vector(unreal.Vector(0, 0, -1)).x, q3.rotate_vector(unreal.Vector(0, 0, -1)).y, q3.rotate_vector(unreal.Vector(0, 0, -1)).z],
    "pommel_dir_in_hand": [q3.rotate_vector(unreal.Vector(0, 0, 1)).x, q3.rotate_vector(unreal.Vector(0, 0, 1)).y, q3.rotate_vector(unreal.Vector(0, 0, 1)).z],
    "crossguard_dir_in_hand": [q3.rotate_vector(unreal.Vector(1, 0, 0)).x, q3.rotate_vector(unreal.Vector(1, 0, 0)).y, q3.rotate_vector(unreal.Vector(1, 0, 0)).z],
    "flat_dir_in_hand": [q3.rotate_vector(unreal.Vector(0, 1, 0)).x, q3.rotate_vector(unreal.Vector(0, 1, 0)).y, q3.rotate_vector(unreal.Vector(0, 1, 0)).z]
}

# Compare with pitch = -90
r4 = unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0)
q4 = r4.quaternion()
out["pitch_neg90"] = {
    "blade_dir_in_hand": [q4.rotate_vector(unreal.Vector(0, 0, -1)).x, q4.rotate_vector(unreal.Vector(0, 0, -1)).y, q4.rotate_vector(unreal.Vector(0, 0, -1)).z],
    "pommel_dir_in_hand": [q4.rotate_vector(unreal.Vector(0, 0, 1)).x, q4.rotate_vector(unreal.Vector(0, 0, 1)).y, q4.rotate_vector(unreal.Vector(0, 0, 1)).z],
    "crossguard_dir_in_hand": [q4.rotate_vector(unreal.Vector(1, 0, 0)).x, q4.rotate_vector(unreal.Vector(1, 0, 0)).y, q4.rotate_vector(unreal.Vector(1, 0, 0)).z],
    "flat_dir_in_hand": [q4.rotate_vector(unreal.Vector(0, 1, 0)).x, q4.rotate_vector(unreal.Vector(0, 1, 0)).y, q4.rotate_vector(unreal.Vector(0, 1, 0)).z]
}

with open(unreal.Paths.project_saved_dir() + "RRTools/test_vector_rotations.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_VECTOR_ROTATIONS_DONE")
