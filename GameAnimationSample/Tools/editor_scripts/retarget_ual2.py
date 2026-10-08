import unreal, json
log = {}
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
GS = "/Game/RR/ExternalAnims/UAL2/UAL2_Standard"
OUT = "/Game/RR/ExternalAnims/UAL2/Retarget"
PLAYER_OUT = "/Game/RR/Player/Anims"
for d in (OUT, "/Game/RR/Player", PLAYER_OUT):
    if not unreal.EditorAssetLibrary.does_directory_exist(d):
        unreal.EditorAssetLibrary.make_directory(d)

gs_mesh = unreal.load_asset(GS + "/SkeletalMeshes/UAL2_Standard")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
log["meshes"] = [str(gs_mesh), str(ue_mesh), str(ue_rig)]

# bones of the Greystone skeleton (for manual chain fallback)
bones = []
try:
    mod = unreal.SkeletonModifier()
    mod.set_skeletal_mesh(gs_mesh)
    bones = [str(b) for b in mod.get_all_bone_names()]
except Exception as e:
    log["bones_err"] = str(e)[:150]
log["greystone_bones"] = bones

# 1) IK rig for Greystone
rig_path = OUT + "/IK_UAL2"
if unreal.EditorAssetLibrary.does_asset_exist(rig_path):
    gs_rig = unreal.load_asset(rig_path)
else:
    gs_rig = at.create_asset("IK_UAL2", OUT, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
ctrl = unreal.IKRigController.get_controller(gs_rig)
log["rig_set_mesh"] = ctrl.set_skeletal_mesh(gs_mesh)
log["rig_auto"] = ctrl.apply_auto_generated_retarget_definition()
chains = ctrl.get_retarget_chains()
log["boneChain_attrs"] = [n for n in dir(chains[0]) if not n.startswith("_")][:30] if chains else []
log["rig_chains_after_auto"] = [str(c.get_editor_property("chain_name")) + ":" + str(ctrl.get_retarget_chain_start_bone(c.get_editor_property("chain_name"))) + "->" + str(ctrl.get_retarget_chain_end_bone(c.get_editor_property("chain_name"))) for c in chains]
if len(chains) < 6 and bones:
    # manual Paragon-style chains
    def has(b): return b in bones
    spec = [("Spine","spine_01","spine_03"),("Neck","neck_01","head"),
            ("LeftArm","upperarm_l","hand_l"),("RightArm","upperarm_r","hand_r"),
            ("LeftLeg","thigh_l","foot_l"),("RightLeg","thigh_r","foot_r")]
    for n,s,e in spec:
        if has(s) and has(e):
            try: ctrl.add_retarget_chain(n, s, e, "")
            except Exception as ex: log["chain_err_"+n] = str(ex)[:100]
    for root in ("pelvis","Pelvis","hips"):
        if has(root):
            ctrl.set_retarget_root(root); break
    log["rig_chains_manual"] = [str(c.get_editor_property("chain_name")) for c in ctrl.get_retarget_chains()]
unreal.EditorAssetLibrary.save_asset(rig_path)

# 2) retargeter Greystone(src) -> UEFN mannequin(target)
rtg_path = OUT + "/RTG_UAL2_to_UEFN"
if unreal.EditorAssetLibrary.does_asset_exist(rtg_path):
    rtg = unreal.load_asset(rtg_path)
else:
    rtg = at.create_asset("RTG_UAL2_to_UEFN", OUT, unreal.IKRetargeter, unreal.IKRetargetFactory())
rc = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
rc.set_ik_rig(S, gs_rig)
rc.set_ik_rig(T, ue_rig)
rc.set_preview_mesh(S, gs_mesh)
rc.set_preview_mesh(T, ue_mesh)
try: rc.add_default_ops()
except Exception as e: log["ops_err"] = str(e)[:150]
try: rc.assign_ik_rig_to_all_ops(S, gs_rig); rc.assign_ik_rig_to_all_ops(T, ue_rig)
except Exception as e: log["assign_err"] = str(e)[:150]
try: rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
except Exception as e: log["map_err"] = str(e)[:150]
try: rc.auto_align_all_bones(S)
except Exception as e: log["align_err"] = str(e)[:150]
log["num_ops"] = rc.get_num_retarget_ops()
unreal.EditorAssetLibrary.save_asset(rtg_path)

# 3) batch retarget combat animations
names = ["UAL2_StandardSword_Block","UAL2_StandardSword_Regular_A","UAL2_StandardSword_Regular_B","UAL2_StandardSword_Regular_C",
         "UAL2_StandardSword_Regular_Combo","UAL2_StandardSword_Dash_RM","UAL2_StandardNinjaJump_Start","UAL2_StandardNinjaJump_Idle_Loop",
         "UAL2_StandardNinjaJump_Land","UAL2_StandardSlide_Start","UAL2_StandardSlide_Loop","UAL2_StandardSlide_Exit","UAL2_StandardHit_Knockback",
         "UAL2_StandardIdle_Shield_Loop","UAL2_StandardShield_Dash_RM","UAL2_StandardMelee_Hook","UAL2_StandardIdle_Shield_Break"]
assets = []
flt = unreal.ARFilter(package_paths=[GS + "/SkeletalMeshes"], recursive_paths=True)
for a in ar.get_assets(flt):
    if str(a.asset_class_path.asset_name) == "AnimSequence" and str(a.asset_name) in names:
        assets.append(a)
log["to_retarget"] = [str(a.asset_name) for a in assets]
batch = unreal.IKRetargetBatchOperation()
try:
    res = batch.duplicate_and_retarget(assets, gs_mesh, ue_mesh, rtg, "", "", "RT_", "", PLAYER_OUT, False, False, True)
    log["retargeted"] = [str(a.asset_name) for a in res]
    log["retargeted_paths"] = [str(a.package_name) for a in res]
    for a in res:
        try: unreal.EditorAssetLibrary.save_asset(str(a.package_name))
        except Exception as ex: log["save_err_" + str(a.asset_name)] = str(ex)[:100]
except Exception as e:
    log["batch_err"] = str(e)[:300]
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
open(unreal.Paths.project_saved_dir() + "RRTools/retarget_ual2_out.json", "w").write(json.dumps(log, indent=1, default=str))
unreal.log("RR_RETARGET_DONE")
