import unreal, json

t_socket = unreal.Transform(
    unreal.Vector(-10.764, 5.811, -0.728),
    unreal.Rotator(-3.176, -180.0, 0.0),
    unreal.Vector(1, 1, 1)
)

out = {}

for p in [-90.0, 90.0, 0.0, 180.0]:
    for y in [0.0, 90.0, -90.0, 180.0]:
        t_sword = unreal.Transform(
            unreal.Vector(0, 0, 0),
            unreal.Rotator(pitch=p, yaw=y, roll=0.0),
            unreal.Vector(1, 1, 1)
        )
        t_comb = unreal.TransformLibrary.compose_transforms(t_sword, t_socket)
        r = t_comb.rotation.rotator()
        out[f"P{int(p)}_Y{int(y)}"] = {
            "loc": [t_comb.translation.x, t_comb.translation.y, t_comb.translation.z],
            "rot": [r.pitch, r.yaw, r.roll]
        }

with open(unreal.Paths.project_saved_dir() + "RRTools/test_transforms.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_TRANSFORMS_DONE")
