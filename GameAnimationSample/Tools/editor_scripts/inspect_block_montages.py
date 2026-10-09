import unreal, json

out = {}

for m_path in ["/Game/RR/Player/AM_P_SwordBlock", "/Game/RR/Player/AM_P_Pro_Block"]:
    m = unreal.load_asset(m_path)
    if not m:
        continue
    name = m.get_name()
    slots = []
    for s in m.get_editor_property("slot_anim_tracks"):
        slot_name = s.slot_name
        slots.append(str(slot_name))
    
    sections = []
    for i in range(m.get_num_sections()):
        sec_name = m.get_section_name(i)
        next_sec = m.get_editor_property("composite_sections")[i].next_section_name if hasattr(m.get_editor_property("composite_sections")[i], "next_section_name") else None
        sections.append({
            "name": str(sec_name),
            "next": str(next_sec)
        })
        
    out[name] = {
        "length": m.get_play_length(),
        "slots": slots,
        "sections": sections
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_block_montages.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BLOCK_MONTAGES_DONE")
