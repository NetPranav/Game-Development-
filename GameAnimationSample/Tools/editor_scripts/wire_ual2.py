import unreal, traceback
at = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
PA = "/Game/RR/Player/Anims/"
PM = "/Game/RR/Player/"

def montage_from(seq_name, name, blend_in=0.08, blend_out=0.2):
    path = PM + name
    if eal.does_asset_exist(path):
        eal.delete_asset(path)
    s = unreal.load_asset(PA + seq_name)
    f = unreal.AnimMontageFactory()
    f.set_editor_property("source_animation", s)
    m = at.create_asset(name, PM.rstrip("/"), unreal.AnimMontage, f)
    m.set_editor_property("blend_in", unreal.AlphaBlend(blend_in=blend_in)) if False else None
    try:
        bi = m.get_editor_property("blend_in"); bi.set_editor_property("blend_time", blend_in); m.set_editor_property("blend_in", bi)
        bo = m.get_editor_property("blend_out"); bo.set_editor_property("blend_time", blend_out); m.set_editor_property("blend_out", bo)
    except Exception as e:
        print("blend err", e)
    eal.save_asset(path)
    print("MONTAGE", path, m.get_editor_property("sequence_length") if hasattr(m, "get_editor_property") else "")
    return m

M = {}
for seq, name, bi, bo in [
    ("RT_UAL2_StandardSword_Block", "AM_P_SwordBlock", 0.06, 0.18),
    ("RT_UAL2_StandardHit_Knockback", "AM_P_Knockback", 0.05, 0.25),
    ("RT_UAL2_StandardSlide_Loop", "AM_P_Slide", 0.1, 0.25),
    ("RT_UAL2_StandardSlide_Start", "AM_P_SlideStart", 0.05, 0.1),
    ("RT_UAL2_StandardNinjaJump_Start", "AM_P_JumpStart", 0.05, 0.1),
    ("RT_UAL2_StandardIdle_Shield_Break", "AM_P_GuardBreak", 0.05, 0.25),
    ("RT_UAL2_StandardSword_Dash_RM", "AM_P_SwordDash", 0.05, 0.2),
]:
    try:
        M[name] = montage_from(seq, name, bi, bo)
    except Exception as e:
        print("ERR", name, e, traceback.format_exc()[-200:])

# player weapon: block + parry (deflect) use the sword block animation
try:
    da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
    anims = da.get_editor_property("animations")
    for prop in ("block_montage", "parry_montage"):
        try:
            anims.set_editor_property(prop, M["AM_P_SwordBlock"])
        except Exception as e:
            print("prop err", prop, e)
    da.set_editor_property("animations", anims)
    eal.save_loaded_asset(da)
    print("DA updated")
except Exception as e:
    print("DA ERR", e)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
print("WIRE_UAL2_DONE")
