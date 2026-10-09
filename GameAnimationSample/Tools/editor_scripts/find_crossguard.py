import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_Sword_A.obj"

wide_verts = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
            if abs(x) > 15.0: # Close to max width 18.68
                wide_verts.append((x, y, z))

ys = [v[1] for v in wide_verts]
print(f"Wide verts count: {len(wide_verts)}, Y range: [{min(ys)}, {max(ys)}]")

# Also let's find the pointiest tip (where width X is closest to 0 at the extreme end)
verts_all = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts_all.append((float(parts[1]), float(parts[2]), float(parts[3])))

ys_all = [v[1] for v in verts_all]
min_y = min(ys_all)
max_y = max(ys_all)

min_y_verts = [v for v in verts_all if v[1] < min_y + 1.0]
max_y_verts = [v for v in verts_all if v[1] > max_y - 1.0]

out = {
    "crossguard_y_range": [min(ys), max(ys)],
    "at_y_min_0": {
        "count": len(min_y_verts),
        "x_range": [min([v[0] for v in min_y_verts]), max([v[0] for v in min_y_verts])],
        "z_range": [min([v[2] for v in min_y_verts]), max([v[2] for v in min_y_verts])]
    },
    "at_y_max_152": {
        "count": len(max_y_verts),
        "x_range": [min([v[0] for v in max_y_verts]), max([v[0] for v in max_y_verts])],
        "z_range": [min([v[2] for v in max_y_verts]), max([v[2] for v in max_y_verts])]
    }
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/crossguard_location.json", "w") as f:
    json.dump(out, f, indent=2)

print("CROSSGUARD_LOCATION_DONE")
