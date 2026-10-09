import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_Sword_A.obj"

verts = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

xs = [v[0] for v in verts]
ys = [v[1] for v in verts]
zs = [v[2] for v in verts]

min_z = min(zs)
max_z = max(zs)

# Sample vertices near min_z (bottom 10%)
near_min = [v for v in verts if v[2] < min_z + (max_z - min_z) * 0.1]
# Sample vertices near max_z (top 10%)
near_max = [v for v in verts if v[2] > max_z - (max_z - min_z) * 0.1]

# Cross-section extents near min vs max
min_xs = [v[0] for v in near_min]
min_ys = [v[1] for v in near_min]
max_xs = [v[0] for v in near_max]
max_ys = [v[1] for v in near_max]

out = {
    "num_verts": len(verts),
    "x_range": [min(xs), max(xs)],
    "y_range": [min(ys), max(ys)],
    "z_range": [min_z, max_z],
    "bottom_10pct": {
        "x_extent": [min(min_xs), max(min_xs)],
        "y_extent": [min(min_ys), max(min_ys)],
        "count": len(near_min),
        "sample": near_min[:5]
    },
    "top_10pct": {
        "x_extent": [min(max_xs), max(max_xs)],
        "y_extent": [min(max_ys), max(max_ys)],
        "count": len(near_max),
        "sample": near_max[:5]
    }
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/analyze_sword_obj.json", "w") as f:
    json.dump(out, f, indent=2)

print("ANALYZE_SWORD_OBJ_DONE")
