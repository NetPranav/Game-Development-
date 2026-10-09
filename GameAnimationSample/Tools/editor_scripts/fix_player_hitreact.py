import unreal
eal = unreal.EditorAssetLibrary
bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
comp = None
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name(): comp = o
kb = unreal.load_asset("/Game/RR/Player/AM_P_Knockback")
kb.set_editor_property("rate_scale", 1.35)
eal.save_loaded_asset(kb)
hr = comp.get_editor_property("hit_reactions")
for k in ("front", "back", "left", "right"):
    hr.set_editor_property(k, kb)   # Greystone hit reacts hunch the robot to the ground; the UAL2 knockback stays upright
comp.set_editor_property("hit_reactions", hr)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
print("HITREACT_DONE")
