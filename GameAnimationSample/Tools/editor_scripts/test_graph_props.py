import unreal

bp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
for g in unreal.BlueprintEditorLibrary.list_graphs(bp):
    if g.get_name() == "UpdateRetargetProfile":
        print("UpdateRetargetProfile class:", g.get_class().get_name())
        try:
            nodes = g.get_editor_property("nodes")
            print("Nodes count:", len(nodes))
        except Exception as e:
            print("Exception reading nodes:", e)
            print("Available properties:", [p for p in dir(g) if not p.startswith("_")])
