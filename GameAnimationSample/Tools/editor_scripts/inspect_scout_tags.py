import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
abp_cdo = unreal.get_default_object(abp.generated_class())

scout_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
scout_cdo = unreal.get_default_object(scout_bp.generated_class())

out = {
    "abp_IKRetargeter": str(abp_cdo.get_editor_property("IKRetargeter")),
    "abp_map": str(abp_cdo.get_editor_property("IKRetargeter_Map")),
    "scout_tags": [str(t) for t in scout_cdo.get_editor_property("tags")]
}

# Mesh component in BP_RobotScout
mesh_comp = scout_cdo.get_editor_property("mesh") if hasattr(scout_cdo, "mesh") else None
if mesh_comp:
    out["mesh_tags"] = [str(t) for t in mesh_comp.get_editor_property("component_tags")]
    sk_mesh = mesh_comp.get_editor_property("skeletal_mesh_asset")
    out["sk_mesh"] = str(sk_mesh.get_path_name()) if sk_mesh else None
    anim_inst = mesh_comp.get_editor_property("anim_class")
    out["anim_class"] = str(anim_inst.get_path_name()) if anim_inst else None

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_scout_tags.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SCOUT_TAGS_DONE")
