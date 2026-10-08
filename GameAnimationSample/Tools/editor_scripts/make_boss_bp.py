import unreal
eal = unreal.EditorAssetLibrary
src = "/Game/RR/Blueprints/BP_RREnemy_Greystone"
dst = "/Game/RR/Blueprints/BP_RRBoss"
if eal.does_asset_exist(dst):
    eal.delete_asset(dst)
eal.duplicate_asset(src, dst)
bp = unreal.load_asset(dst)
ok = unreal.BlueprintEditorLibrary.reparent_blueprint(bp, unreal.load_class(None, "/Script/RRGame.RRBoss"))
print("REPARENT", ok)
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if isinstance(o, unreal.SkeletalMeshComponent):
        o.set_editor_property("relative_scale3d", unreal.Vector(1.2, 1.2, 1.2))
        print("scaled mesh")
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
print("BOSS_BP_DONE")
