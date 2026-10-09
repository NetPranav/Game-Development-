import unreal, json

out = {}

# Check player character anim blueprint and default state
bp = unreal.load_asset("/Game/AAA/Blueprints/Characters/BP_PlayerCharacter")
cdo = unreal.get_default_object(bp.generated_class())
mesh = cdo.get_editor_property("mesh")
anim_class = mesh.get_editor_property("anim_class")
out["anim_class"] = str(anim_class.get_path_name()) if anim_class else None

# Check the robot scout visual override
robot_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
robot_cdo = unreal.get_default_object(robot_bp.generated_class())
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = sub.k2_gather_subobject_data_for_blueprint(robot_bp)
for h in handles:
    d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
    o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
    if isinstance(o, unreal.SkeletalMeshComponent):
        out["robot_mesh_anim_class"] = str(o.get_editor_property("anim_class").get_path_name()) if o.get_editor_property("anim_class") else None
        out["robot_mesh_skel"] = str(o.get_editor_property("skeletal_mesh_asset").get_path_name()) if o.get_editor_property("skeletal_mesh_asset") else None

# Inspect ABP_GenericRetarget
abp = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/ABP_GenericRetarget")
abp_cdo = unreal.get_default_object(abp.generated_class())
out["abp_retargeter"] = str(abp_cdo.get_editor_property("IKRetargeter").get_path_name()) if hasattr(abp_cdo, "IKRetargeter") and abp_cdo.get_editor_property("IKRetargeter") else None

# Check all animations in /Game/Characters/Mannequins/Animations/ or player anims
# What is the default idle animation in the game?
# In GameAnimationSample (GASP):
# ABP is ABP_SandboxCharacter / ABP_Manny
out["player_mesh"] = str(mesh.get_editor_property("skeletal_mesh_asset").get_path_name()) if mesh.get_editor_property("skeletal_mesh_asset") else None

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_anim_setup.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ANIM_SETUP_DONE")
