import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
def montage(seq_path, name, folder, rate, bi=0.15, bo=0.25):
    path = folder + "/" + name
    m = unreal.load_asset(path) if eal.does_asset_exist(path) else None
    if not m:
        f = unreal.AnimMontageFactory(); f.set_editor_property("source_animation", unreal.load_asset(seq_path))
        m = at.create_asset(name, folder, unreal.AnimMontage, f)
    m.set_editor_property("rate_scale", rate)
    for attr, t in (("blend_in", bi), ("blend_out", bo)):
        b = m.get_editor_property(attr); b.set_editor_property("blend_time", t); m.set_editor_property(attr, b)
    eal.save_loaded_asset(m)
    print("W1", path, "len %.2f" % m.get_play_length())
    return m
A = "/Game/RR/NPC/Anims/RT_UAL1_AL_StandardRig_"
montage(A + "Dance_Loop", "AM_NPC_Dance", "/Game/RR/NPC", 1.0, 0.3, 0.4)
montage(A + "Idle_Talking_Loop", "AM_NPC_Talk", "/Game/RR/NPC", 1.0, 0.3, 0.4)
chest = montage(A + "Hit_Chest", "AM_P_HitChest", "/Game/RR/Player", 0.8, 0.06, 0.2)
head = montage(A + "Hit_Head", "AM_P_HitHead", "/Game/RR/Player", 0.8, 0.06, 0.2)
montage(A + "Roll", "AM_P_Roll", "/Game/RR/Player", 1.15, 0.08, 0.2)

bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
comp = None
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name(): comp = o
hr = comp.get_editor_property("hit_reactions")
hr.set_editor_property("front", chest); hr.set_editor_property("back", head)
hr.set_editor_property("left", head); hr.set_editor_property("right", chest)
comp.set_editor_property("hit_reactions", hr)
unreal.BlueprintEditorLibrary.compile_blueprint(bp); eal.save_loaded_asset(bp)
# boss: which montages does its combat component use for reactions, and on which slot?
bb = unreal.load_asset("/Game/RR/Blueprints/BP_RRBoss")
for h in sub.k2_gather_subobject_data_for_blueprint(bb):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name():
        r = o.get_editor_property("hit_reactions")
        for k in ("front", "back", "left", "right", "stagger", "death", "knockdown", "get_up"):
            m = r.get_editor_property(k)
            print("BOSSHR", k, m.get_path_name() if m else None, [str(t.get_editor_property("slot_name")) for t in m.get_editor_property("slot_anim_tracks")] if m else "")
print("W1_DONE")
