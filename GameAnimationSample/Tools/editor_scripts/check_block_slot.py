import unreal, json

m = unreal.load_asset("/Game/RR/Player/AM_P_SwordBlock")
out = {
    "num_sections": m.get_num_sections(),
    "sec_0": str(m.get_section_name(0)),
    "slots": [str(t.slot_name) for t in m.get_editor_property("slot_anim_tracks")]
}
with open(unreal.Paths.project_saved_dir() + "RRTools/block_slot.json", "w") as f:
    json.dump(out, f, indent=2)

print("BLOCK_SLOT_DONE")
