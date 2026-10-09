import unreal, json

eal = unreal.EditorAssetLibrary
out = {}

# 1. Inspect RTG_Pro_to_UEFN
rtg_path = "/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN"
rtg = unreal.load_asset(rtg_path)
if rtg:
    rc = unreal.IKRetargeterController.get_controller(rtg)
    chains = {}
    chain_names = [
        "Root", "Pelvis", "Spine", "Head",
        "LeftClavicle", "LeftArm", "LeftForearm", "LeftHand",
        "RightClavicle", "RightArm", "RightForearm", "RightHand",
        "LeftThigh", "LeftLeg", "LeftFoot", "LeftToe",
        "RightThigh", "RightLeg", "RightFoot", "RightToe"
    ]
    for c_name in chain_names:
        try:
            src_chain = rc.get_source_chain(c_name)
            chains[c_name] = str(src_chain)
        except Exception as e:
            chains[c_name] = f"ERR: {e}"
    out["rtg_chains"] = chains
else:
    out["rtg_chains"] = "NOT FOUND"

# 2. Inspect Montages
montages = [
    "/Game/RR/Player/AM_P_Pro_L1",
    "/Game/RR/Player/AM_P_Pro_L2",
    "/Game/RR/Player/AM_P_Pro_L3",
    "/Game/RR/Player/AM_P_Pro_L4",
    "/Game/RR/Player/AM_P_Pro_L5",
    "/Game/RR/Player/AM_P_Pro_Block",
    "/Game/RR/Player/AM_P_SwordBlock",
    "/Game/RR/Player/AM_P_UAL_L1",
    "/Game/RR/Player/AM_P_UAL_L2",
    "/Game/RR/Player/AM_P_UAL_L3"
]

montage_info = {}
for m_path in montages:
    m = unreal.load_asset(m_path)
    if m:
        slots = []
        try:
            for track in m.get_editor_property("slot_anim_tracks"):
                slots.append(str(track.get_editor_property("slot_name")))
        except Exception as e:
            slots.append(f"ERR: {e}")
        sections = []
        try:
            for sec in m.get_editor_property("composite_sections"):
                sections.append(str(sec.get_editor_property("section_name")))
        except Exception as e:
            sections.append(f"ERR: {e}")
        
        seq_name = "None"
        try:
            tracks = m.get_editor_property("slot_anim_tracks")
            if len(tracks) > 0:
                anim_track = tracks[0].get_editor_property("anim_track")
                segs = anim_track.get_editor_property("anim_segments")
                if len(segs) > 0:
                    seq_name = str(segs[0].get_editor_property("anim_reference").get_path_name())
        except Exception as e:
            seq_name = f"ERR: {e}"

        montage_info[m.get_name()] = {
            "slots": slots,
            "sections": sections,
            "length": m.get_play_length(),
            "sequence": seq_name
        }
    else:
        montage_info[m_path] = "NOT FOUND"
out["montages"] = montage_info

# 3. Check SandboxCharacter_Mover_ABP slots
abp_path = "/Game/Blueprints/SandboxCharacter_Mover_ABP"
abp = unreal.load_asset(abp_path)
out["player_abp_found"] = abp is not None

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/current_anim_state.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_STATE_DONE")
