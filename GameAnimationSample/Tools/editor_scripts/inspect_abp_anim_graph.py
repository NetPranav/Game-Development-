import unreal, json

abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

# In AnimBlueprint, let's look for nodes or anim graphs
try:
    for g in abp.get_editor_property("anim_graph_pages") if hasattr(abp, "anim_graph_pages") else []:
        out[str(g.get_name())] = [str(n.get_name()) for n in g.get_editor_property("nodes")]
except Exception as e:
    out["anim_graph_err"] = str(e)

# Let's search inside the package of ABP_GenericRetarget for all objects
pkg = abp.get_package()
obj_list = []
for obj in unreal.PackageTools.get_objects_in_package(pkg) if hasattr(unreal, "PackageTools") else []:
    obj_list.append(f"{obj.get_class().get_name()}:{obj.get_name()}")
out["package_objects"] = obj_list

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_anim_graph.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_ANIM_GRAPH_DONE")
