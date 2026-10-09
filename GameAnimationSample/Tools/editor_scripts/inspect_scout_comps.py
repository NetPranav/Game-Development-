import unreal, json

bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
cdo = unreal.get_default_object(bp.generated_class())

comp_info = []
# Find all subobjects of CDO
subobjs = unreal.PackageTools.get_objects_in_package(bp.get_package()) if hasattr(unreal, "PackageTools") else []
for obj in subobjs:
    if isinstance(obj, unreal.SkeletalMeshComponent):
        comp_info.append({
            "name": obj.get_name(),
            "tags": [str(t) for t in obj.get_editor_property("component_tags")],
            "sk_mesh": str(obj.get_editor_property("skeletal_mesh_asset").get_path_name()) if obj.get_editor_property("skeletal_mesh_asset") else None,
            "anim_class": str(obj.get_editor_property("anim_class").get_path_name()) if obj.get_editor_property("anim_class") else None
        })

with open(unreal.Paths.project_saved_dir() + "RRTools/scout_comps.json", "w") as f:
    json.dump(comp_info, f, indent=2)

print("SCOUT_COMPS_DONE")
