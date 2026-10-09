import unreal
eal = unreal.EditorAssetLibrary
def bp_template(bp, class_substr):
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    for h in sub.k2_gather_subobject_data_for_blueprint(bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if obj and class_substr in obj.get_class().get_name():
            return obj
    return None
# 1) player stagger: the Greystone "Stun" kneels on the ground -> use the standing guard-break
bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
comp = bp_template(bp, "AAACombatComponent")
hr = comp.get_editor_property("hit_reactions")
hr.set_editor_property("stagger", unreal.load_asset("/Game/RR/Player/AM_P_GuardBreak"))
comp.set_editor_property("hit_reactions", hr)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
print("STAGGER_SET")
# 2) heavy attack: sword dash slash, in place (the RM version would drag the mesh away from the capsule)
seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Dash_RM")
try:
    seq.set_editor_property("enable_root_motion", False)
    eal.save_loaded_asset(seq)
except Exception as e:
    print("rm err", e)
m = unreal.load_asset("/Game/RR/Player/AM_P_SwordDash")
m.set_editor_property("rate_scale", 1.15)
eal.save_loaded_asset(m)
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
anims = da.get_editor_property("animations")
anims.set_editor_property("heavy_attack_combos", [m])
da.set_editor_property("animations", anims)
eal.save_loaded_asset(da)
print("FIX_PLAYER_DONE")
