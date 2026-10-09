import unreal
# Montages from the repaired Pro Sword & Shield clips (RT_PSS_*) and wiring of the player's weapon / hit reactions.
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
A = "/Game/RR/Player/Anims/PSS/RT_PSS_"
OUT = "/Game/RR/Player/PSS"

def montage(clip, name, rate, bi=0.12, bo=0.22):
    path = OUT + "/" + name
    if eal.does_asset_exist(path):
        eal.delete_asset(path)
    f = unreal.AnimMontageFactory(); f.set_editor_property("source_animation", unreal.load_asset(A + clip))
    m = at.create_asset(name, OUT, unreal.AnimMontage, f)
    m.set_editor_property("rate_scale", rate)
    for attr, t in (("blend_in", bi), ("blend_out", bo)):
        b = m.get_editor_property(attr); b.set_editor_property("blend_time", t); m.set_editor_property(attr, b)
    eal.save_loaded_asset(m)
    print("PSSW", name, "<-", clip, "len %.2f" % m.get_play_length())
    return m

L1 = montage("Slash", "AM_PSS_L1", 1.3)
L2 = montage("Slash3", "AM_PSS_L2", 1.3)
L3 = montage("Attack4", "AM_PSS_L3", 1.15)
H1 = montage("Attack3", "AM_PSS_H1", 1.15, 0.15, 0.25)
H2 = montage("Kick", "AM_PSS_H2", 1.1)
H3 = montage("Attack2", "AM_PSS_H3", 1.1, 0.15, 0.25)
BLK = montage("Block", "AM_PSS_Block", 1.0, 0.06, 0.2)
PAR = montage("Block2", "AM_PSS_Parry", 1.25, 0.04, 0.2)
EQ = montage("DrawSword1", "AM_PSS_Draw", 1.2, 0.1, 0.2)
UNEQ = montage("SheathSword2", "AM_PSS_Sheath", 1.2, 0.1, 0.25)
IMP_F = montage("Impact2", "AM_PSS_ImpactFront", 1.1, 0.05, 0.2)
IMP_B = montage("Impact3", "AM_PSS_ImpactSide", 1.1, 0.05, 0.2)
STAG = montage("Impact", "AM_PSS_Stagger", 0.8, 0.08, 0.3)
DEATH = montage("Death", "AM_PSS_Death", 1.0, 0.1, 0.0)
montage("PowerUp", "AM_PSS_PowerUp", 1.0)
montage("Casting", "AM_PSS_Casting", 1.0)

da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
an = da.get_editor_property("animations")
an.set_editor_property("light_attack_combos", [L1, L2, L3])
an.set_editor_property("heavy_attack_combos", [H1, H2, H3])
an.set_editor_property("block_montage", BLK)
an.set_editor_property("parry_montage", PAR)
an.set_editor_property("equip_animation", EQ)
an.set_editor_property("unequip_animation", UNEQ)
da.set_editor_property("animations", an)
eal.save_loaded_asset(da)

bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name():
        r = o.get_editor_property("hit_reactions")
        r.set_editor_property("front", IMP_F); r.set_editor_property("back", IMP_B)
        r.set_editor_property("left", IMP_B); r.set_editor_property("right", IMP_F)
        r.set_editor_property("stagger", STAG); r.set_editor_property("death", DEATH)
        o.set_editor_property("hit_reactions", r)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
print("PSSW_DONE")
