import unreal, json
log = {}
at = unreal.AssetToolsHelpers.get_asset_tools(); eal = unreal.EditorAssetLibrary
def mk(seq, name):
    path = "/Game/RR/Player/" + name
    if eal.does_asset_exist(path): return unreal.load_asset(path)
    f = unreal.AnimMontageFactory(); f.set_editor_property("source_animation", unreal.load_asset("/Game/RR/Player/Anims/" + seq))
    m = at.create_asset(name, "/Game/RR/Player", unreal.AnimMontage, f); eal.save_asset(path); return m
res = {}
for seq, name in (("RT_Attack_D_Slow", "AM_P_Heavy2"), ("RT_Attack_C_Slow", "AM_P_Heavy3")):
    m = mk(seq, name)
    bi = m.get_editor_property("blend_in"); bi.set_editor_property("blend_time", 0.1); m.set_editor_property("blend_in", bi)
    bo = m.get_editor_property("blend_out"); bo.set_editor_property("blend_time", 0.25); m.set_editor_property("blend_out", bo)
    eal.save_loaded_asset(m)
    res[name] = m.get_editor_property("sequence_length")
log["lengths"] = res
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
anims = da.get_editor_property("animations")
anims.set_editor_property("heavy_attack_combos", [unreal.load_asset("/Game/RR/Player/AM_P_Heavy2")])
da.set_editor_property("animations", anims)
eal.save_asset("/Game/RR/Player/DA_PlayerWeapon")
st = unreal.load_asset("/Game/RR/Player/AM_P_Stun"); st.set_editor_property("rate_scale", 2.2); eal.save_loaded_asset(st)
open(unreal.Paths.project_saved_dir() + "RRTools/heavy_out.json", "w").write(json.dumps(log))
