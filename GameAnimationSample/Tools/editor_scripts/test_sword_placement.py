import unreal, json

# Load world
world = unreal.EditorLevelLibrary.get_editor_world()

# Load robot BP
bp_class = unreal.load_class(None, "/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")
weapon_da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")

out = {}

# Test various candidate HandOffset transforms relative to hand_r:
# Candidate 1: Epic's standard weapon_r socket transform
# Loc = (-10.764, 5.811, -0.728), Rot = (-3.176, -180.0, 0.0)
# Candidate 2: Blade pointing forward / down
# Let's inspect the world transform of hand_r on a spawned robot actor

actor = unreal.EditorLevelLibrary.spawn_actor_from_class(bp_class, unreal.Vector(0, 0, 100))
try:
    mesh = actor.find_component_by_class(unreal.SkeletalMeshComponent)
    if mesh:
        hand_t = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_WORLD)
        out["hand_r_world_loc"] = [hand_t.translation.x, hand_t.translation.y, hand_t.translation.z]
        r = hand_t.rotation.rotator()
        out["hand_r_world_rot"] = [r.pitch, r.yaw, r.roll]
        
        # Test candidate offsets:
        test_offsets = {
            "epic_weapon_r": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
            "pitch_neg90": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
            "pitch_pos90": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
            "yaw_neg90": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
            "yaw_pos90": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
            "identity": unreal.Transform(
                unreal.Vector(-10.764, 5.811, -0.728),
                unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0),
                unreal.Vector(1, 1, 1)
            ),
        }
        
        # For each offset, compute where the blade tip (Z=140 relative to weapon origin) ends up in world space
        for name, off in test_offsets.items():
            # In UE, relative transform composed with hand_r:
            # actor_world = off * hand_t
            # Let's compute world transform
            w_rot = hand_t.rotation * off.rotation
            w_loc = hand_t.translation + hand_t.rotation.rotate_vector(off.translation)
            
            # Tip vector in weapon space: (0, 0, 140)
            tip_w = w_loc + w_rot.rotate_vector(unreal.Vector(0, 0, 140))
            
            out[name] = {
                "weapon_grip_loc": [w_loc.x, w_loc.y, w_loc.z],
                "blade_tip_loc": [tip_w.x, tip_w.y, tip_w.z],
                "tip_relative_to_grip": [tip_w.x - w_loc.x, tip_w.y - w_loc.y, tip_w.z - w_loc.z]
            }
finally:
    actor.destroy_actor()

with open(unreal.Paths.project_saved_dir() + "RRTools/test_sword_placement.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_SWORD_PLACEMENT_DONE")
