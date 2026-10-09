import unreal, json

AL = unreal.AnimationLibrary

src_seq = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/Pro_Slash_1")
out = {}
if src_seq:
    for b in ["Hips", "Spine", "Spine1", "Spine2", "RightUpLeg", "LeftUpLeg"]:
        t0 = AL.get_bone_pose_for_frame(src_seq, b, 0, False)
        r = t0.rotation.rotator()
        out[b] = {
            "loc": [t0.translation.x, t0.translation.y, t0.translation.z],
            "rot": [r.pitch, r.yaw, r.roll]
        }
    sk = src_seq.get_editor_property("skeleton")
    ref = sk.get_reference_pose()
    ref_poses = {}
    for b in ["Hips", "Spine", "Spine1", "Spine2", "RightUpLeg", "LeftUpLeg"]:
        rb = ref.get_ref_bone_pose(b)
        r = rb.rotation.rotator()
        ref_poses[b] = {
            "loc": [rb.translation.x, rb.translation.y, rb.translation.z],
            "rot": [r.pitch, r.yaw, r.roll]
        }
    out["ref_pose"] = ref_poses

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_mixamo_src.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_MIXAMO_SRC_DONE")
