import unreal

def comps(bp_path):
    bp = unreal.load_asset(bp_path)
    print("BP", bp_path, bp)
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sub.k2_gather_subobject_data_for_blueprint(bp)
    for h in handles:
        d = sub.k2_find_subobject_data_from_handle(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        print("  comp", o.get_class().get_name(), o.get_name())
        if isinstance(o, unreal.SkeletalMeshComponent):
            print("     mesh", o.get_editor_property("skeletal_mesh"))
            print("     anim_class", o.get_editor_property("anim_class"))
            print("     anim_mode", o.get_editor_property("animation_mode"))
            print("     rel loc", o.get_editor_property("relative_location"), "rot", o.get_editor_property("relative_rotation"))
        if isinstance(o, unreal.CharacterMovementComponent):
            print("     max_walk", o.get_editor_property("max_walk_speed"))

comps("/Game/RR/Blueprints/BP_RREnemy_Greystone")
comps("/Game/RR/Blueprints/BP_RobotScout")

# robot skeleton bones
for p in unreal.EditorAssetLibrary.list_assets("/Game", recursive=True):
    if "robot" in p.lower() and "SK_" in p and "Skeleton" not in p:
        a = unreal.load_asset(p.split(".")[0])
        if isinstance(a, unreal.SkeletalMesh):
            print("ROBOT MESH", p)
            names = [str(n) for n in a.skeleton.get_editor_property("bone_tree")] if False else None
            sk = a.get_editor_property("skeleton")
            print("  skeleton", sk)
            try:
                bones = unreal.AnimationLibrary.get_skeleton_bone_names(sk) if hasattr(unreal.AnimationLibrary, "get_skeleton_bone_names") else []
            except Exception as e:
                bones = []
            print("  bones", [str(b) for b in bones][:80])
