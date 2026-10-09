import json

obj_path = "a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/SM_Sword_A.obj"

verts = []
# Unreal OBJ exporter:
# In Unreal coords (X, Y, Z):
# In OBJ: line is 'v X Z Y' or 'v X Y Z'
# In our previous script:
# x_range: [-18.68, 18.68]
# y_range: [0.0, 152.29] -> this corresponds to Unreal Z (height 152.3)!
# z_range: [-7.57, 7.57] -> this corresponds to Unreal Y (thickness 7.57)!

with open(obj_path, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            # parts[1] is X, parts[2] is length (Unreal Z), parts[3] is thickness (Unreal Y)
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

# Let's inspect length slices:
# Slice 1: length in [0, 15] (Unreal Z ~ 0 to 15)
slice_pommel_side = [v for v in verts if v[1] < 15.0]
# Slice 2: length in [15, 30] (Unreal Z ~ 15 to 30)
slice_crossguard = [v for v in verts if 15.0 <= v[1] <= 30.0]
# Slice 3: length in [135, 152.3] (Unreal Z ~ 135 to 152.3)
slice_tip_side = [v for v in verts if v[1] > 135.0]

def extents(slice_verts):
    xs = [v[0] for v in slice_verts]
    zs = [v[2] for v in slice_verts]
    return {
        "count": len(slice_verts),
        "x_range": [min(xs), max(xs)] if xs else [],
        "thickness_range": [min(zs), max(zs)] if zs else []
    }

out = {
    "length_0_to_15": extents(slice_pommel_side),
    "length_15_to_30": extents(slice_crossguard),
    "length_135_to_152": extents(slice_tip_side)
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/slice_analysis.json", "w") as f:
    json.dump(out, f, indent=2)

print("SLICE_ANALYSIS_DONE")
