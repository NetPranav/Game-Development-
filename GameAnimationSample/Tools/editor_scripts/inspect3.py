import unreal, json
out = {}
try:
    unreal.EditorLoadingAndSavingUtils.load_map("/Game/AAA/Levels/L_Developer")
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    rows = []
    for a in actors:
        loc = a.get_actor_location()
        rows.append("%s | %s | %.0f,%.0f,%.0f" % (a.get_class().get_name(), a.get_actor_label(), loc.x, loc.y, loc.z))
    out["L_Developer_actor_count"] = len(rows)
    out["L_Developer_actors"] = sorted(rows)[:80]
    ws = unreal.EditorLevelLibrary.get_editor_world().get_world_settings()
    out["world_settings_gm"] = str(ws.get_editor_property("default_game_mode"))
except Exception as e:
    out["level_err"] = str(e)[:300]

# BP_PlayerCharacter: find CombatComponent template and print its editable props
try:
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
    handles = sub.k2_gather_subobject_data_for_blueprint(bp)
    found = []
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if obj and "CombatComponent" in obj.get_class().get_name():
            found.append(obj.get_path_name())
            try:
                out["combat_hit_reactions"] = str(obj.get_editor_property("hit_reactions"))[:200]
            except Exception as e:
                out["combat_hit_reactions_err"] = str(e)[:150]
    out["combat_templates"] = found
except Exception as e:
    out["bp_err"] = str(e)[:300]

# Greystone ABP generated class + mesh property names on ACharacter CDO
try:
    abp = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint")
    out["abp_gen_class"] = str(abp.generated_class())
    cdo = unreal.get_default_object(unreal.Character)
    m = cdo.get_editor_property("mesh")
    out["mesh_props"] = [n for n in dir(m) if "anim" in n.lower() or "skeletal_mesh" in n.lower()][:30]
except Exception as e:
    out["abp_err"] = str(e)[:300]
out["rr_classes"] = [n for n in dir(unreal) if n.startswith("RR")]
open(unreal.Paths.project_saved_dir() + "RRTools/inspect3_out.json", "w").write(json.dumps(out, indent=1, default=str))
unreal.log("RR_INSPECT3_DONE")
