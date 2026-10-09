import unreal, json

eal = unreal.EditorAssetLibrary
AL = unreal.AnimationLibrary

report = {}

# 1. RTG check
rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)
report["rtg_root_source_chain"] = str(rc.get_source_chain("Root"))
report["rtg_root_is_unmapped"] = (report["rtg_root_source_chain"] == "None" or report["rtg_root_source_chain"] == "")

# 2. Montages check
montages = ["AM_P_Pro_L1", "AM_P_Pro_L2", "AM_P_Pro_L3", "AM_P_Pro_Block"]
m_info = {}
for m_name in montages:
    m = unreal.load_asset(f"/Game/RR/Player/{m_name}")
    if m:
        seq_ref = "None"
        tracks = m.get_editor_property("slot_anim_tracks")
        if tracks:
            segs = tracks[0].get_editor_property("anim_track").get_editor_property("anim_segments")
            if segs:
                seq_ref = str(segs[0].get_editor_property("anim_reference").get_name())
        m_info[m_name] = {
            "found": True,
            "play_length": round(m.get_play_length(), 3),
            "sequence": seq_ref
        }
    else:
        m_info[m_name] = {"found": False}
report["montages"] = m_info

# 3. DA_PlayerWeapon check
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    anims = da.get_editor_property("animations")
    report["da_weapon"] = {
        "light_combos": [str(m.get_name()) for m in anims.get_editor_property("light_attack_combos")],
        "heavy_combos": [str(m.get_name()) for m in anims.get_editor_property("heavy_attack_combos")],
        "block": str(anims.get_editor_property("block_montage").get_name()) if anims.get_editor_property("block_montage") else "None",
        "parry": str(anims.get_editor_property("parry_montage").get_name()) if anims.get_editor_property("parry_montage") else "None"
    }

# 4. Bone pose verification on the 3 combo slashes
poses = {}
for anim_name in ["RT_Pro_Slash_1", "RT_Pro_Slash_2", "RT_Pro_Slash_3"]:
    seq = unreal.load_asset(f"/Game/RR/Player/Anims/{anim_name}")
    if seq:
        t0 = AL.get_bone_pose_for_frame(seq, "root", 0, False)
        t_pelv = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
        poses[anim_name] = {
            "root_translation": [round(t0.translation.x, 2), round(t0.translation.y, 2), round(t0.translation.z, 2)],
            "root_rotation": [round(t0.rotation.rotator().pitch, 2), round(t0.rotation.rotator().yaw, 2), round(t0.rotation.rotator().roll, 2)],
            "pelvis_height": round(t_pelv.translation.z, 2)
        }
report["slash_poses"] = poses

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/final_verification.json", "w") as f:
    json.dump(report, f, indent=2)

print("VERIFICATION_COMPLETE")
