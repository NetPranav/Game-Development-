import unreal

eal = unreal.EditorAssetLibrary
mesh_path = "/Game/RR/Environment/Colosseum/SM_Colosseum"
mesh = unreal.load_asset(mesh_path)

if mesh:
    # 1. Enable Nanite
    nanite_settings = mesh.get_editor_property("nanite_settings")
    nanite_settings.set_editor_property("enabled", True)
    mesh.set_editor_property("nanite_settings", nanite_settings)
    
    # 2. Set Collision: Use Complex Collision as Simple so player can walk on the exact 3D geometry
    body_setup = mesh.get_editor_property("body_setup")
    if body_setup:
        body_setup.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    
    # 3. Save asset
    eal.save_loaded_asset(mesh)
    print("SUCCESS: Enabled Nanite and complex-as-simple collision on SM_Colosseum!")
else:
    print(f"ERROR: Could not load {mesh_path}")

# Fix PlayerStart in L_RRArena so it's elevated above the floor
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
acts = eas.get_all_level_actors()

ps_list = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
if ps_list:
    ps = ps_list[0]
    loc = ps.get_actor_location()
    # Lift PlayerStart Z well above ground (e.g. Z = 300) so capsule does not intersect floor
    ps.set_actor_location(unreal.Vector(loc.x, loc.y, max(loc.z, 300.0)), False, False)
    print(f"SUCCESS: Adjusted PlayerStart to location {ps.get_actor_location()}")
    unreal.EditorLoadingAndSavingUtils.save_current_level()

print("COLOSSEUM_AND_SPAWN_FIX_COMPLETE")
