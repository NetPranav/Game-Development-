import unreal, json

mesh = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")

# Let's inspect materials and LOD info
out = {}
mats = []
for i in range(mesh.get_num_sections(0)):
    mats.append(str(mesh.get_material(i)))
out["sections"] = mats

# Let's check if we can inspect vertices via Unreal Python
# If not, let's export as OBJ/FBX or check with EditorStaticMeshLibrary
try:
    # unreal.EditorStaticMeshLibrary
    # Let's check methods of EditorStaticMeshLibrary
    methods = dir(unreal.EditorStaticMeshLibrary)
    out["mesh_lib_methods"] = [m for m in methods if not m.startswith("_")]
except Exception as e:
    out["err"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_sword_geom.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SWORD_GEOM_DONE")
