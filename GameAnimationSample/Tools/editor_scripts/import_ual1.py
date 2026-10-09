import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
FBX = "A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalAnims/UAL1/ex/Animation Library[Standard]/Unreal Engine/AL_Standard.fbx"
DST = "/Game/RR/ExternalAnims/UAL1"
OUT = "/Game/RR/NPC/Anims"
WANT = ["Dance_Loop", "Idle_Talking_Loop", "Hit_Chest", "Hit_Head", "Roll", "Sitting_Idle_Loop", "Interact"]

mesh = None
for a in ar.get_assets(unreal.ARFilter(package_paths=[DST], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "SkeletalMesh")])):
    mesh = a.get_asset()
if not mesh:
    t = unreal.AssetImportTask(); t.filename = FBX; t.destination_path = DST; t.automated = True; t.save = True; t.replace_existing = True
    ui = unreal.FbxImportUI(); ui.import_mesh = True; ui.import_animations = True; ui.import_materials = False; ui.import_textures = False
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    t.options = ui
    at.import_asset_tasks([t])
    print("UAL1 imported", len(t.imported_object_paths))
    for p in t.imported_object_paths:
        o = unreal.load_asset(p)
        if isinstance(o, unreal.SkeletalMesh): mesh = o
print("UAL1 mesh", mesh.get_path_name() if mesh else None)

anims = {}
for a in ar.get_assets(unreal.ARFilter(package_paths=[DST], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")])):
    anims[str(a.asset_name).replace("AL_StandardRig_", "")] = a
print("UAL1 anim names", sorted(anims.keys())[:60])

# IK rig + retargeter (same recipe as retarget_ual2.py)
rig_path = DST + "/IK_UAL1"
rig = unreal.load_asset(rig_path) if eal.does_asset_exist(rig_path) else at.create_asset("IK_UAL1", DST, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
rc = unreal.IKRigController.get_controller(rig)
rc.set_skeletal_mesh(mesh); rc.apply_auto_generated_retarget_definition()
print("UAL1 chains", [str(c.get_editor_property("chain_name")) for c in rc.get_retarget_chains()], "root", rc.get_retarget_root())
eal.save_asset(rig_path)
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
rtg_path = DST + "/RTG_UAL1_to_UEFN"
rtg = unreal.load_asset(rtg_path) if eal.does_asset_exist(rtg_path) else at.create_asset("RTG_UAL1_to_UEFN", DST, unreal.IKRetargeter, unreal.IKRetargetFactory())
c = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
c.set_ik_rig(S, rig); c.set_ik_rig(T, ue_rig); c.set_preview_mesh(S, mesh); c.set_preview_mesh(T, ue_mesh)
if c.get_num_retarget_ops() == 0:
    c.add_default_ops()
c.assign_ik_rig_to_all_ops(S, rig); c.assign_ik_rig_to_all_ops(T, ue_rig)
c.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
eal.save_asset(rtg_path)

todo = [anims[n] for n in WANT if n in anims and not eal.does_asset_exist(OUT + "/RT_UAL1_" + n)]
if todo:
    res = unreal.IKRetargetBatchOperation().duplicate_and_retarget(todo, mesh, ue_mesh, rtg, "", "", "RT_UAL1_", "", OUT, False, False, True)
    for a in res: eal.save_asset(str(a.package_name))
    print("UAL1 retargeted", [str(a.asset_name) for a in res])
AL = unreal.AnimationLibrary
for n in WANT:
    s = unreal.load_asset(OUT + "/RT_UAL1_" + n)
    if s:
        p = AL.get_bone_pose_for_frame(s, "pelvis", 0, False).translation
        th = AL.get_bone_pose_for_frame(s, "thigh_l", 0, False).translation
        print("UAL1 check %s pelvis=(%.0f,%.0f,%.0f) thighL=(%.1f,%.1f,%.1f) len=%.2f" % (n, p.x, p.y, p.z, th.x, th.y, th.z, s.get_play_length()))
print("UAL1_DONE")
