import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_LongSword.obj"

verts = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

xs = [v[0] for v in verts]
ys = [v[1] for v in verts]
zs = [v[2] for v in verts]

min_x, max_x = min(xs), max(xs)
min_y, max_y = min(ys), max(ys)
min_z, max_z = min(zs), max(zs)

# Check crossguard (widest point in Y or Z)
wide_y = [v for v in verts if abs(v[1]) > (max_y - min_y)*0.35]
wide_z = [v for v in verts if abs(v[2]) > (max_z - min_z)*0.35]

out = {
    "num_verts": len(verts),
    "x_range": [min_x, max_x],
    "y_range": [min_y, max_y],
    "z_range": [min_z, max_z],
    "wide_in_z_x_range": [min([v[0] for v in wide_z]), max([v[0] for v in wide_z])] if wide_z else None,
    "at_min_x": [v for v in verts if v[0] < min_x + 2.0][:5],
    "at_max_x": [v for v in verts if v[0] > max_x - 2.0][:5]
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/analyze_longsword.json", "w") as f:
    json.dump(out, f, indent=2)

print("ANALYZE_LONGSWORD_COMPLETE")
