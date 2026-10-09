import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_Sword_A.obj"

verts = []
with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

handle_verts = [v for v in verts if 116.0 <= v[1] <= 145.0]
pommel_verts = [v for v in verts if v[1] > 145.0]

out = {
    "handle_116_to_145": {
        "count": len(handle_verts),
        "x_range": [min([v[0] for v in handle_verts]), max([v[0] for v in handle_verts])],
        "z_range": [min([v[2] for v in handle_verts]), max([v[2] for v in handle_verts])],
    },
    "pommel_145_to_152": {
        "count": len(pommel_verts),
        "x_range": [min([v[0] for v in pommel_verts]), max([v[0] for v in pommel_verts])],
        "z_range": [min([v[2] for v in pommel_verts]), max([v[2] for v in pommel_verts])],
    }
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/handle_check.json", "w") as f:
    json.dump(out, f, indent=2)

print("HANDLE_CHECK_DONE")
