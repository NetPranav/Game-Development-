import unreal, json
out = {}
def props(obj, names):
    r = {}
    for n in names:
        try: r[n] = str(obj.get_editor_property(n))
        except Exception as e: r[n] = "ERR:" + str(e)[:80]
    return r
ar = unreal.AssetRegistryHelpers.get_asset_registry()
def find(cls_name, path):
    flt = unreal.ARFilter(package_paths=[path], recursive_paths=True)
    return [a for a in ar.get_assets(flt) if str(a.asset_class_path.asset_name) == cls_name]
out["counts"] = {}
for c in ["AnimMontage","AnimSequence","Blueprint","GameplayEffect","SkeletalMesh","StaticMesh"]:
    out["counts"][c] = {p: len(find(c, p)) for p in ["/Game/ParagonGreystone","/Game/AAA","/Game/Blueprints","/Game/Characters"]}
out["greystone_montages"] = [str(a.asset_name) for a in find("AnimMontage","/Game/ParagonGreystone")]
out["greystone_skm"] = [str(a.package_name) for a in find("SkeletalMesh","/Game/ParagonGreystone")]
out["greystone_abp"] = [str(a.package_name) for a in find("AnimBlueprint","/Game/ParagonGreystone")]
out["aaa_blueprints"] = [str(a.package_name) for a in find("Blueprint","/Game/AAA")]
out["gameplay_effects"] = [str(a.package_name) for a in find("Blueprint","/Game/AAA")]
out["npc_like_bps"] = [str(a.package_name) for a in find("Blueprint","/Game/Blueprints") if any(k in str(a.asset_name) for k in ["NPC","Sandbox","Mover","Enemy","AI"])]
try:
    sw = unreal.load_asset("/Game/AAA/DataAssets/Items/DA_TestSword")
    out["DA_TestSword_class"] = str(sw.get_class().get_name())
    out["DA_TestSword"] = props(sw, ["weapon_type","visuals","combat","abilities","animations"])
except Exception as e:
    out["DA_TestSword"] = "ERR:" + str(e)[:200]
try:
    ge = unreal.load_asset("/Game/AAA/Gameplay/Effects/GE_DirectDamage")
    cdo = unreal.get_default_object(ge.generated_class())
    out["GE_DirectDamage"] = props(cdo, ["duration_policy","modifiers","executions","gameplay_effect_tags"] ) if hasattr(ge,"generated_class") else str(ge)
except Exception as e:
    out["GE_DirectDamage"] = "ERR:" + str(e)[:200]
path = unreal.Paths.project_saved_dir() + "RRTools/inspect_out.json"
open(path, "w").write(json.dumps(out, indent=1, default=str))
unreal.log("RR_INSPECT_DONE " + path)
