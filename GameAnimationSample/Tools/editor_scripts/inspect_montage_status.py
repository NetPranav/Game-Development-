import unreal, json

eal = unreal.EditorAssetLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
AL = unreal.AnimationLibrary

res = {}

montages = [
    "/Game/RR/Player/AM_P_Pro_L1",
    "/Game/RR/Player/AM_P_Pro_L2",
    "/Game/RR/Player/AM_P_Pro_L3",
    "/Game/RR/Player/AM_P_Pro_Block",
    "/Game/RR/Player/AM_P_SwordBlock"
]

for m_path in montages:
    m = unreal.load_asset(m_path)
    if not m:
        res[m_path] = "NOT_FOUND"
        continue
    
    slots = []
    anim_refs = []
    for track in m.get_editor_property("slot_anim_tracks"):
        slot_name = str(track.slot_name)
        slots.append(slot_name)
        anim_track = track.get_editor_property("anim_track")
        for seg in anim_track.get_editor_property("anim_segments"):
            anim_ref = seg.get_editor_property("anim_reference")
            anim_refs.append(anim_ref.get_path_name() if anim_ref else "None")
            
    sections = []
    for i in range(m.get_num_sections()):
        sec_name = str(m.get_section_name(i))
        next_sec = ""
        try:
            cs = m.get_editor_property("composite_sections")
            if i < len(cs):
                next_sec = str(cs[i].next_section_name)
        except Exception as e:
            next_sec = str(e)
        sections.append({"name": sec_name, "next": next_sec})
        
    res[m_path] = {
        "exists": True,
        "play_length": m.get_play_length(),
        "slots": slots,
        "anim_refs": anim_refs,
        "sections": sections,
        "blend_in": m.get_editor_property("blend_in").get_editor_property("blend_time"),
        "blend_out": m.get_editor_property("blend_out").get_editor_property("blend_time"),
        "rate_scale": m.get_editor_property("rate_scale")
    }

# Also inspect DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    anims = da.get_editor_property("animations")
    l_combos = [a.get_path_name() if a else "None" for a in anims.get_editor_property("light_attack_combos")]
    blk = anims.get_editor_property("block_montage")
    res["DA_PlayerWeapon"] = {
        "light_attack_combos": l_combos,
        "block_montage": blk.get_path_name() if blk else "None"
    }

# Also check RT_Pro_Slash_1, RT_Pro_Slash_2, RT_Pro_Slash_3, RT_Pro_Block
for a_name in ["RT_Pro_Slash_1", "RT_Pro_Slash_2", "RT_Pro_Slash_3", "RT_Pro_Block"]:
    seq = unreal.load_asset(f"/Game/RR/Player/Anims/{a_name}")
    if seq:
        t0_root = AL.get_bone_pose_for_frame(seq, "root", 0, False)
        t0_pelvis = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
        mid_root = AL.get_bone_pose_for_frame(seq, "root", AL.get_num_frames(seq) // 2, False)
        mid_pelvis = AL.get_bone_pose_for_frame(seq, "pelvis", AL.get_num_frames(seq) // 2, False)
        res[a_name] = {
            "num_frames": AL.get_num_frames(seq),
            "length": AL.get_sequence_length(seq),
            "t0_root_loc": [t0_root.translation.x, t0_root.translation.y, t0_root.translation.z],
            "t0_root_rot": [t0_root.rotation.rotator().pitch, t0_root.rotation.rotator().yaw, t0_root.rotation.rotator().roll],
            "t0_pelvis_loc": [t0_pelvis.translation.x, t0_pelvis.translation.y, t0_pelvis.translation.z],
            "t0_pelvis_rot": [t0_pelvis.rotation.rotator().pitch, t0_pelvis.rotation.rotator().yaw, t0_pelvis.rotation.rotator().roll],
            "mid_root_loc": [mid_root.translation.x, mid_root.translation.y, mid_root.translation.z],
            "mid_root_rot": [mid_root.rotation.rotator().pitch, mid_root.rotation.rotator().yaw, mid_root.rotation.rotator().roll],
            "mid_pelvis_loc": [mid_pelvis.translation.x, mid_pelvis.translation.y, mid_pelvis.translation.z],
            "mid_pelvis_rot": [mid_pelvis.rotation.rotator().pitch, mid_pelvis.rotation.rotator().yaw, mid_pelvis.rotation.rotator().roll]
        }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/montage_and_anim_status.json", "w") as f:
    json.dump(res, f, indent=2)

print("INSPECT_MONTAGE_STATUS_COMPLETE")
