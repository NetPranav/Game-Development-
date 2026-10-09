import unreal, json

AL = unreal.AnimationLibrary

seq = unreal.load_asset("/Game/RR/Player/Anims/RT_Pro_Slash_1")
out = {}
if seq:
    n = AL.get_num_frames(seq)
    out["num_frames"] = n
    out["rate"] = AL.get_sequence_length(seq)
    
    bones_to_check = ["root", "pelvis", "spine_01", "spine_02", "spine_03", "thigh_l", "thigh_r", "clavicle_r", "upperarm_r"]
    poses = {}
    for b in bones_to_check:
        t0 = AL.get_bone_pose_for_frame(seq, b, 0, False)
        poses[b] = {
            "loc": [t0.translation.x, t0.translation.y, t0.translation.z],
            "rot": [t0.rotation.rotator().pitch, t0.rotation.rotator().yaw, t0.rotation.rotator().roll]
        }
    out["frame_0"] = poses

    sk = seq.get_editor_property("skeleton")
    ref = sk.get_reference_pose()
    ref_poses = {}
    for b in bones_to_check:
        rb = ref.get_ref_bone_pose(b)
        ref_poses[b] = {
            "loc": [rb.translation.x, rb.translation.y, rb.translation.z],
            "rot": [rb.rotation.rotator().pitch, rb.rotation.rotator().yaw, rb.rotation.roll if hasattr(rb.rotation, "roll") else 0.0]
        }
    out["ref_pose"] = ref_poses

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_pro_slash1.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_PRO_SLASH1_DONE")
