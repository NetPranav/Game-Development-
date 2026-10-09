import unreal, os, re
# Pro Sword & Shield pack (Mixamo): import all 52 animations onto the pack skeleton, retarget to the UEFN mannequin
# (the player's master skeleton) into /Game/RR/Player/Anims/PSS as RT_PSS_<name>. Idempotent; run pss_repair.py after.
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
SRC_DIR = "A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalAnims/ProSwordShield/"
SRC_PKG = "/Game/RR/ExternalAnims/ProSwordShield/All"
OUT = "/Game/RR/Player/Anims/PSS"
skel_mesh = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/SKM_ProSwordShield_Base")
skeleton = skel_mesh.get_editor_property("skeleton")

def clean(n):
    n = n.replace("sword and shield ", "").replace(".fbx", "")
    n = re.sub(r"[^A-Za-z0-9]+", "_", n).strip("_")
    return n.title().replace("_", "")

# 1) import animations only, onto the existing pack skeleton
tasks = []
for f in sorted(os.listdir(SRC_DIR)):
    if not f.lower().endswith(".fbx") or f.startswith("Ch01"):
        continue
    name = "PSS_" + clean(f)
    if eal.does_asset_exist(SRC_PKG + "/" + name):
        continue
    t = unreal.AssetImportTask(); t.filename = SRC_DIR + f; t.destination_path = SRC_PKG; t.destination_name = name
    t.automated = True; t.save = True; t.replace_existing = True
    ui = unreal.FbxImportUI()
    ui.import_mesh = False; ui.import_animations = True; ui.import_materials = False; ui.import_textures = False
    ui.skeleton = skeleton
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    ad = ui.anim_sequence_import_data
    ad.set_editor_property("import_bone_tracks", True)
    ad.set_editor_property("remove_redundant_keys", False)
    t.options = ui
    tasks.append(t)
if tasks:
    at.import_asset_tasks(tasks)
print("PSS imported", sum(len(t.imported_object_paths) for t in tasks), "new")

ar = unreal.AssetRegistryHelpers.get_asset_registry()
src = [a for a in ar.get_assets(unreal.ARFilter(package_paths=[SRC_PKG], recursive_paths=True, class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "AnimSequence")]))]
print("PSS source anims", len(src))

# 2) fresh IK rig + retargeter (auto-generated Mixamo chains -> UEFN)
rig_path = SRC_PKG + "/IK_PSS"
rig = unreal.load_asset(rig_path) if eal.does_asset_exist(rig_path) else at.create_asset("IK_PSS", SRC_PKG, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
rc = unreal.IKRigController.get_controller(rig)
rc.set_skeletal_mesh(skel_mesh)
rc.apply_auto_generated_retarget_definition()
print("PSS rig root", rc.get_retarget_root(), "chains", [str(c.get_editor_property("chain_name")) for c in rc.get_retarget_chains()])
eal.save_asset(rig_path)
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
rtg_path = SRC_PKG + "/RTG_PSS_to_UEFN"
rtg = unreal.load_asset(rtg_path) if eal.does_asset_exist(rtg_path) else at.create_asset("RTG_PSS_to_UEFN", SRC_PKG, unreal.IKRetargeter, unreal.IKRetargetFactory())
c = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
c.set_ik_rig(S, rig); c.set_ik_rig(T, ue_rig); c.set_preview_mesh(S, skel_mesh); c.set_preview_mesh(T, ue_mesh)
if c.get_num_retarget_ops() == 0:
    c.add_default_ops()
c.assign_ik_rig_to_all_ops(S, rig); c.assign_ik_rig_to_all_ops(T, ue_rig)
c.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
try:
    c.auto_align_all_bones(S)
except Exception as e:
    print("PSS align err", e)
print("PSS ops", [c.get_op_name(i) for i in range(c.get_num_retarget_ops())])
eal.save_asset(rtg_path)

# 3) batch retarget everything that is not done yet
todo = [a for a in src if not eal.does_asset_exist(OUT + "/RT_" + str(a.asset_name))]
if todo:
    res = unreal.IKRetargetBatchOperation().duplicate_and_retarget(todo, skel_mesh, ue_mesh, rtg, "", "", "RT_", "", OUT, False, False, True)
    for a in res:
        eal.save_asset(str(a.package_name))
    print("PSS retargeted", len(res))
print("PSS_DONE")
