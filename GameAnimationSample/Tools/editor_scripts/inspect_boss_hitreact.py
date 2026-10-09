import unreal, json

out = {}

# Check BP_RRBoss
boss_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RRBoss")
if not boss_bp:
    boss_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RREnemy_Greystone")

if boss_bp:
    cdo = unreal.get_default_object(boss_bp.generated_class())
    out["boss_class"] = boss_bp.get_name()
    
    # Check CombatComponent
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sub.k2_gather_subobject_data_for_blueprint(boss_bp)
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if isinstance(o, unreal.SkeletalMeshComponent):
            out["mesh"] = str(o.get_editor_property("skeletal_mesh_asset"))
            out["anim_class"] = str(o.get_editor_property("anim_class"))
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

# Also check BP_RREnemy_Greystone
grey_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RREnemy_Greystone")
if grey_bp:
    for h in sub.k2_gather_subobject_data_for_blueprint(grey_bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if "CombatComponent" in o.get_class().get_name():
            hr = o.get_editor_property("hit_reactions")
            out["grey_hit_reactions"] = {
                "front": str(hr.get_editor_property("front")),
                "back": str(hr.get_editor_property("back")),
                "left": str(hr.get_editor_property("left")),
                "right": str(hr.get_editor_property("right")),
                "stagger": str(hr.get_editor_property("stagger")),
                "death": str(hr.get_editor_property("death")),
            }

# Check montage slots for AM_GS_HitReact_Front
m_front = unreal.load_asset("/Game/RR/Greystone/AM_GS_HitReact_Front")
if m_front:
    out["m_front_skeleton"] = str(m_front.get_editor_property("skeleton"))
    slots = []
    for track in m_front.get_editor_property("slot_anim_tracks"):
        slots.append(str(track.get_editor_property("slot_name")))
    out["m_front_slots"] = slots

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_boss_hitreact.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BOSS_HITREACT_DONE")
