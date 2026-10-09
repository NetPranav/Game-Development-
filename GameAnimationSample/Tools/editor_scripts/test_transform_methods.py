import unreal, json

t = unreal.Transform()
out = [m for m in dir(t) if not m.startswith("__")]

with open(unreal.Paths.project_saved_dir() + "RRTools/transform_methods.json", "w") as f:
    json.dump(out, f, indent=2)

print("TRANSFORM_METHODS_DONE")
