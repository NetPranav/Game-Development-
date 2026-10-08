import unreal, json
out = {}
ar = unreal.AssetRegistryHelpers.get_asset_registry()
def find(cls, path):
    flt = unreal.ARFilter(package_paths=[path], recursive_paths=True)
    return [a for a in ar.get_assets(flt) if str(a.asset_class_path.asset_name) == cls]
def cdo_props(path, names):
    r = {}
    try:
        bp = unreal.load_asset(path)
        cls = bp.generated_class()
        r["_parent"] = str(unreal.SystemLibrary.get_class_display_name(cls.get_super_class())) if hasattr(cls, "get_super_class") else "?"
        obj = unreal.get_default_object(cls)
        for n in names:
            try: r[n] = str(obj.get_editor_property(n))[:600]
            except Exception as e: r[n] = "ERR " + str(e)[:60]
    except Exception as e:
        r["ERR"] = str(e)[:200]
    return r
def comps(path):
    try:
        sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        bp = unreal.load_asset(path)
        handles = sub.k2_gather_subobject_data_for_blueprint(bp)
        res = []
        for h in handles:
            d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
            res.append(str(obj.get_class().get_name()) + ":" + str(obj.get_name()))
        return res
    except Exception as e:
        return ["ERR " + str(e)[:150]]
out["greystone_anims"] = sorted(str(a.asset_name) for a in find("AnimSequence", "/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations"))[:200]
out["BP_PlayerCharacter_comps"] = comps("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
out["BP_PlayerCharacter_cdo"] = cdo_props("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter", ["character_settings"])
out["DA_TestSword_cdo"] = cdo_props("/Game/AAA/DataAssets/Items/DA_TestSword", ["weapon_type","visuals","combat","abilities","animations"])
out["BP_AAAGameMode_cdo"] = cdo_props("/Game/AAA/Gameplay/GameModes/BP_AAAGameMode", ["default_pawn_class","player_controller_class","hud_class","player_state_class","game_state_class"])
out["Greystone_ABP"] = {}
try:
    abp = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint")
    out["Greystone_ABP"]["skeleton"] = str(abp.get_editor_property("target_skeleton"))
    out["Greystone_ABP"]["parent"] = str(abp.get_editor_property("parent_class"))
except Exception as e:
    out["Greystone_ABP"]["ERR"] = str(e)[:150]
out["sandbox_npc"] = [str(a.package_name) for a in find("Blueprint", "/Game/Blueprints") if "NPC" in str(a.asset_name) or "AI" in str(a.package_name)]
out["blueprints_all"] = sorted(str(a.asset_name) for a in find("Blueprint", "/Game/Blueprints"))
out["levels"] = [str(a.package_name) for a in find("World", "/Game")]
out["char_montage_names"] = sorted(str(a.asset_name) for a in find("AnimMontage", "/Game/Characters"))[:80]
out["paragon_hero_dirs"] = [str(p) for p in unreal.EditorAssetLibrary.list_assets("/Game/ParagonGreystone/Characters/Heroes/Greystone", recursive=False, include_folder=True)][:40]
path = unreal.Paths.project_saved_dir() + "RRTools/inspect2_out.json"
open(path, "w").write(json.dumps(out, indent=1, default=str))
unreal.log("RR_INSPECT2_DONE")
