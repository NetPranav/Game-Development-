import unreal, json, traceback
log = {}
eal = unreal.EditorAssetLibrary
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
def comps(bp):
    res = []
    for h in sub.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o: res.append((o.get_class().get_name(), o.get_name(), o))
    return res
try:
    bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    robot_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
    for cls, name, o in comps(bp):
        if "VisualOverrideManager" in cls:
            log["mgr_before"] = str(o.get_editor_property("VisualOverride"))
            o.set_editor_property("VisualOverride", robot_bp.generated_class())
            log["mgr_after"] = str(o.get_editor_property("VisualOverride"))
        if cls == "SkeletalMeshComponent" and name == "SkeletalMesh_GEN_VARIABLE":
            o.set_editor_property("hidden_in_game", False)   # manager hides it itself when an override is active
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)
except Exception as e:
    log["ERR"] = str(e)[:300] + traceback.format_exc()[-300:]
open(unreal.Paths.project_saved_dir() + "RRTools/wire_robot2_out.json", "w").write(json.dumps(log, indent=1))
