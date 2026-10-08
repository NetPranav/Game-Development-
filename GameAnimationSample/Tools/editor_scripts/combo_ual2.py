import unreal
at = unreal.AssetToolsHelpers.get_asset_tools(); eal = unreal.EditorAssetLibrary
def mk(seq, name, bi, bo, rate):
    path = "/Game/RR/Player/" + name
    if eal.does_asset_exist(path):
        m = unreal.load_asset(path)
    else:
        f = unreal.AnimMontageFactory(); f.set_editor_property("source_animation", unreal.load_asset("/Game/RR/Player/Anims/" + seq))
        m = at.create_asset(name, "/Game/RR/Player", unreal.AnimMontage, f)
    b = m.get_editor_property("blend_in"); b.set_editor_property("blend_time", bi); m.set_editor_property("blend_in", b)
    b = m.get_editor_property("blend_out"); b.set_editor_property("blend_time", bo); m.set_editor_property("blend_out", b)
    m.set_editor_property("rate_scale", rate)
    eal.save_loaded_asset(m)
    print("LEN", name, m.get_editor_property("sequence_length"))
    return m
a = mk("RT_UAL2_StandardSword_Regular_A", "AM_P_UAL_L1", 0.06, 0.15, 0.85)
b = mk("RT_UAL2_StandardSword_Regular_B", "AM_P_UAL_L2", 0.06, 0.15, 0.85)
c = mk("RT_UAL2_StandardSword_Regular_C", "AM_P_UAL_L3", 0.06, 0.2, 1.6)
h = mk("RT_UAL2_StandardMelee_Hook", "AM_P_UAL_Heavy", 0.08, 0.2, 0.6)
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
anims = da.get_editor_property("animations")
anims.set_editor_property("light_attack_combos", [a, b, c])
anims.set_editor_property("heavy_attack_combos", [h])
da.set_editor_property("animations", anims)
eal.save_loaded_asset(da)
print("COMBO_UAL2_DONE")
