import unreal, json
at = unreal.AssetToolsHelpers.get_asset_tools(); eal = unreal.EditorAssetLibrary
def mk(seq, name, bi, bo, rate):
    path = "/Game/RR/Player/" + name
    if eal.does_asset_exist(path): m = unreal.load_asset(path)
    else:
        f = unreal.AnimMontageFactory(); f.set_editor_property("source_animation", unreal.load_asset("/Game/RR/Player/Anims/" + seq))
        m = at.create_asset(name, "/Game/RR/Player", unreal.AnimMontage, f)
    b = m.get_editor_property("blend_in"); b.set_editor_property("blend_time", bi); m.set_editor_property("blend_in", b)
    b = m.get_editor_property("blend_out"); b.set_editor_property("blend_time", bo); m.set_editor_property("blend_out", b)
    m.set_editor_property("rate_scale", rate)
    eal.save_loaded_asset(m); return m
f1 = mk("RT_Attack_A_Fast", "AM_P_LightF1", 0.05, 0.15, 1.0)
f2 = mk("RT_Attack_B_Fast", "AM_P_LightF2", 0.05, 0.15, 1.0)
l3 = unreal.load_asset("/Game/RR/Player/AM_P_Light3"); b = l3.get_editor_property("blend_in"); b.set_editor_property("blend_time", 0.06); l3.set_editor_property("blend_in", b); l3.set_editor_property("rate_scale", 1.25); eal.save_loaded_asset(l3)
hv = mk("RT_Attack_B_Med", "AM_P_Heavy4", 0.08, 0.2, 0.95)
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
anims = da.get_editor_property("animations")
anims.set_editor_property("light_attack_combos", [f1, f2, l3])
anims.set_editor_property("heavy_attack_combos", [hv])
da.set_editor_property("animations", anims)
eal.save_asset("/Game/RR/Player/DA_PlayerWeapon")
open(unreal.Paths.project_saved_dir() + "RRTools/combo_out.json", "w").write(json.dumps({"lens": [round(m.get_editor_property("sequence_length"), 2) for m in (f1, f2, l3, hv)]}))
