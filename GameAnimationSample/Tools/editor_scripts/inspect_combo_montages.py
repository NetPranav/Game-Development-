import unreal, json

out = {}
for name in ["AM_P_UAL_L1", "AM_P_UAL_L2", "AM_P_UAL_L3", "AM_P_Pro_L1", "AM_P_Pro_L2", "AM_P_Pro_L3"]:
    m = unreal.load_asset(f"/Game/RR/Player/{name}")
    if not m:
        continue
    out[name] = {
        "length": m.get_play_length(),
        "slots": [str(t.slot_name) for t in m.get_editor_property("slot_anim_tracks")]
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_combo_montages.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_COMBO_MONTAGES_DONE")
