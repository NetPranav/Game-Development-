import unreal, json

robot_mesh = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
seq = unreal.load_asset("/Game/RR/Player/Anims/RT_UAL2_StandardSword_Regular_A")

chain = ["root", "pelvis", "spine_01", "spine_02", "spine_03", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r"]

# Build component space transform by accumulating parent transforms
# In UE, child_cs = child_local.multiply(parent_cs)
comp_transform = unreal.Transform()
for b in chain:
    try:
        t = unreal.AnimationLibrary.get_bone_pose_for_frame(seq, b, 0, False)
        comp_transform = t.multiply(comp_transform)
    except Exception as e:
        print(f"Error on bone {b}: {e}")

hand_cs = comp_transform
grip_loc = unreal.Vector(-10.764, 5.811, -0.728)

candidates = {
    "epic_weapon_r": unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0),
    "epic_yaw_0": unreal.Rotator(pitch=-3.176, yaw=0.0, roll=0.0),
    "pitch_neg90": unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0),
    "pitch_pos90": unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
    "pitch_pos90_yaw180": unreal.Rotator(pitch=90.0, yaw=180.0, roll=0.0),
    "pitch_neg90_yaw180": unreal.Rotator(pitch=-90.0, yaw=180.0, roll=0.0),
    "yaw_neg90": unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0),
    "yaw_pos90": unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0),
    "yaw_180": unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0),
    "zero": unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0),
    "roll_pos90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=90.0),
    "roll_neg90": unreal.Rotator(pitch=0.0, yaw=0.0, roll=-90.0),
}

out = {
    "hand_cs": {
        "loc": [hand_cs.translation.x, hand_cs.translation.y, hand_cs.translation.z],
        "rot": [hand_cs.rotation.rotator().pitch, hand_cs.rotation.rotator().yaw, hand_cs.rotation.rotator().roll]
    },
    "results": {}
}

for name, rot in candidates.items():
    w_rel = unreal.Transform(grip_loc, rot, unreal.Vector(1, 1, 1))
    w_cs = w_rel.multiply(hand_cs)
    
    # Blade tip is at (0, 0, 140) in weapon local space
    tip_rel = unreal.Transform(unreal.Vector(0, 0, 140), unreal.Rotator(), unreal.Vector(1, 1, 1))
    tip_cs = tip_rel.multiply(w_cs)
    
    dir_vec = tip_cs.translation - w_cs.translation
    
    out["results"][name] = {
        "rot": [rot.pitch, rot.yaw, rot.roll],
        "grip_loc_cs": [w_cs.translation.x, w_cs.translation.y, w_cs.translation.z],
        "tip_loc_cs": [tip_cs.translation.x, tip_cs.translation.y, tip_cs.translation.z],
        "blade_vector_cs": [dir_vec.x, dir_vec.y, dir_vec.z]
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/test_sword_pose_calc.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_SWORD_POSE_CALC_DONE")
