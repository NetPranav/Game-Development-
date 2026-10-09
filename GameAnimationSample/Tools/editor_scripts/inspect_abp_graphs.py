import unreal, json

bp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
out = {}

# We can search through graphs in the blueprint
for g in bp.get_editor_property("uber_graph_pages"):
    out[str(g.get_name())] = [str(n.get_name()) for n in g.get_editor_property("nodes")]

for g in bp.get_editor_property("function_graphs") if hasattr(bp, 'function_graphs') else []:
    out[str(g.get_name())] = [str(n.get_name()) for n in g.get_editor_property("nodes")]

# Let's inspect all sub-objects of bp
sub_objs = unreal.EditorFilterLibrary.by_class(unreal.PackageTools.get_objects_in_package(bp.get_package()), unreal.EdGraph) if hasattr(unreal, 'PackageTools') else []

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_abp_graphs.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_ABP_GRAPHS_DONE")
