import unreal, json

boss_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RRBoss")
cdo = unreal.get_default_object(boss_bp.generated_class())

out = {}

# Check Tuning
tuning = cdo.get_editor_property("tuning")
out["tuning"] = {
    "walk_speed": tuning.get_editor_property("walk_speed"),
    "chase_speed": tuning.get_editor_property("chase_speed"),
    "attack_range": tuning.get_editor_property("attack_range"),
    "block_chance": tuning.get_editor_property("block_chance_when_player_attacking"),
}

# Check Greystone_AnimBlueprint
abp = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint")
abp_cdo = unreal.get_default_object(abp.generated_class())

# Get all properties of abp_cdo
props = []
for p in dir(abp_cdo):
    if not p.startswith("_"):
        props.append(p)
out["abp_props"] = [p for p in props if p in ["speed", "is_accelerating", "is_in_air", "direction", "pitch", "yaw"]]

# Check anim graph or variables
vars_info = {}
for prop_name in ["Speed", "IsAccelerating", "IsInAir", "Direction", "Yaw"]:
    try:
        val = abp_cdo.get_editor_property(prop_name)
        vars_info[prop_name] = str(val)
    except Exception as e:
        vars_info[prop_name] = f"Err: {e}"
out["abp_vars"] = vars_info

# Check attacks on boss
attacks = []
for a in cdo.get_editor_property("attacks"):
    attacks.append({
        "id": str(a.get_editor_property("id")),
        "montage": str(a.get_editor_property("montage")),
        "sequence": str(a.get_editor_property("sequence")),
        "range": a.get_editor_property("range"),
        "min_phase": a.get_editor_property("min_phase"),
    })
out["attacks"] = attacks

# Check combat component
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sub.k2_gather_subobject_data_for_blueprint(boss_bp)
for h in handles:
    d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
    if "CombatComponent" in o.get_class().get_name():
        hr = o.get_editor_property("hit_reactions")
        out["hit_reactions"] = {
            "front": str(hr.get_editor_property("front")),
            "back": str(hr.get_editor_property("back")),
            "left": str(hr.get_editor_property("left")),
            "right": str(hr.get_editor_property("right")),
            "stagger": str(hr.get_editor_property("stagger")),
            "knockdown": str(hr.get_editor_property("knockdown")),
            "get_up": str(hr.get_editor_property("get_up")),
            "death": str(hr.get_editor_property("death")),
        }
        out["montage_slot_override"] = str(o.get_editor_property("montage_slot_override"))

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_boss_details.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BOSS_DETAILS_DONE")
