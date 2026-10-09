import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_LongSword.obj"

verts = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

wide_y = [v for v in verts if abs(v[1]) > 8.0]
crossguard_x = [v[0] for v in wide_y]
print(f"Crossguard along X: min={min(crossguard_x)}, max={max(crossguard_x)}")

# Handle is between pommel (x = -75) and crossguard
# Let's inspect vertices between x = -70 and min(crossguard_x)
handle_verts = [v for v in verts if -70.0 < v[0] < min(crossguard_x)]
print(f"Handle verts count: {len(handle_verts)}")
handle_xs = [v[0] for v in handle_verts]
print(f"Handle X range: [{min(handle_xs)}, {max(handle_xs)}]")
handle_mid_x = (min(handle_xs) + max(handle_xs)) / 2.0
print(f"Handle center X: {handle_mid_x}")

# Thickness of blade in Z:
blade_verts = [v for v in verts if v[0] > max(crossguard_x) + 5.0]
blade_zs = [v[2] for v in blade_verts]
print(f"Blade Z range (flat of blade / thickness): [{min(blade_zs)}, {max(blade_zs)}]")
blade_ys = [v[1] for v in blade_verts]
print(f"Blade Y range (width / cutting edges): [{min(blade_ys)}, {max(blade_ys)}]")

