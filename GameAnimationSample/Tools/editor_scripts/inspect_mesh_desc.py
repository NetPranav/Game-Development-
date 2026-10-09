import unreal, json

mesh = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
out = {}

# Check StaticMeshDescription or vertex positions
try:
    sub = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    md = sub.get_mesh_description(mesh, 0)
    # If md exists, let's see its methods/properties
    out["md_type"] = str(type(md))
    out["md_dir"] = [d for d in dir(md) if not d.startswith("_")]
except Exception as e:
    out["err_md"] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_mesh_desc.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_MESH_DESC_DONE")
