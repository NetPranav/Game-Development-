import unreal, json, traceback
log = {}
eal = unreal.EditorAssetLibrary
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)

def step(name):
    def deco(fn):
        try:
            r = fn(); log[name] = "ok" if r is None else r
        except Exception as e:
            log[name] = "ERR " + str(e)[:300] + " | " + traceback.format_exc()[-300:]
        return fn
    return deco

def comps(bp):
    res = []
    for h in sub.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o: res.append((o.get_class().get_name(), o.get_name(), o))
    return res

ROBOT_BP = "/Game/RR/Blueprints/BP_RobotScout"

@step("robot_visual_bp")
def _():
    if not eal.does_asset_exist(ROBOT_BP):
        eal.duplicate_asset("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin", ROBOT_BP)
    bp = unreal.load_asset(ROBOT_BP)
    robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
    n = 0
    for cls, name, o in comps(bp):
        if cls == "SkeletalMeshComponent":
            o.set_editor_property("skeletal_mesh_asset", robot)
            n += 1
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)
    return "meshes set: %d" % n

@step("player_uses_robot")
def _():
    bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    robot_bp = unreal.load_asset(ROBOT_BP)
    done = []
    for cls, name, o in comps(bp):
        if cls == "ChildActorComponent" and "VisualOverride" in name:
            o.set_editor_property("child_actor_class", robot_bp.generated_class())
            done.append("child:" + name)
        if cls == "SkeletalMeshComponent" and name == "SkeletalMesh_GEN_VARIABLE":
            # keep evaluating the pose (the retarget display actor reads it) but do not draw the mannequin
            o.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
            o.set_editor_property("hidden_in_game", True)
            done.append("hid:" + name)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)
    return done

open(unreal.Paths.project_saved_dir() + "RRTools/wire_robot_out.json", "w").write(json.dumps(log, indent=1, default=str))
