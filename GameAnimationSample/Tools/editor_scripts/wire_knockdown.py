import unreal
# Knockdown + get-up for the player (Sekiro: normal hits flinch, perilous hits throw you down and you climb back up).
# 1) retarget UAL2 LayToIdle to UEFN  2) AM_P_GetUp  3) BP_PlayerCharacter hit reactions.
# Run fix_ual2_pelvis.py and fix_ual2_inplace.py after step 1 (this script does step 1 only if needed; run order in HOWTO).
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
SRC = "/Game/RR/ExternalAnims/UAL2/UAL2_Standard/SkeletalMeshes"
OUT = "/Game/RR/Player/Anims"
RT = OUT + "/RT_UAL2_StandardLayToIdle"

if not eal.does_asset_exist(RT):
    gs_mesh = unreal.load_asset(SRC + "/UAL2_Standard")
    ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
    rtg = unreal.load_asset("/Game/RR/ExternalAnims/UAL2/Retarget/RTG_UAL2_to_UEFN")
    assets = [a for a in ar.get_assets(unreal.ARFilter(package_paths=[SRC], recursive_paths=True)) if str(a.asset_name) == "UAL2_StandardLayToIdle"]
    res = unreal.IKRetargetBatchOperation().duplicate_and_retarget(assets, gs_mesh, ue_mesh, rtg, "", "", "RT_", "", OUT, False, False, True)
    for a in res:
        eal.save_asset(str(a.package_name))
    print("KD retargeted", [str(a.asset_name) for a in res])

if eal.does_asset_exist("/Game/RR/Player/AM_P_GetUp"):
    m = unreal.load_asset("/Game/RR/Player/AM_P_GetUp")
else:
    f = unreal.AnimMontageFactory()
    f.set_editor_property("source_animation", unreal.load_asset(RT))
    m = at.create_asset("AM_P_GetUp", "/Game/RR/Player", unreal.AnimMontage, f)
m.set_editor_property("rate_scale", 1.1)
for attr, t in (("blend_in", 0.1), ("blend_out", 0.25)):
    b = m.get_editor_property(attr); b.set_editor_property("blend_time", t); m.set_editor_property(attr, b)
eal.save_loaded_asset(m)
print("KD montage", m.get_path_name(), "len", m.get_play_length())

bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
comp = None
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if o and "AAACombatComponent" in o.get_class().get_name():
        comp = o
flinch = unreal.load_asset("/Game/RR/Player/AM_P_GuardBreak")   # standing recoil
hr = comp.get_editor_property("hit_reactions")
for k in ("front", "back", "left", "right"):
    hr.set_editor_property(k, flinch)
hr.set_editor_property("knockdown", unreal.load_asset("/Game/RR/Player/AM_P_Knockback"))
hr.set_editor_property("get_up", m)
comp.set_editor_property("hit_reactions", hr)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
print("KD_DONE")
