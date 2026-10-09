import unreal, json

AL = unreal.AnimationLibrary

out = {}

for m_name in ["AM_P_SwordBlock", "AM_P_UAL_L1", "AM_P_UAL_L2", "AM_P_UAL_L3", "AM_P_Pro_Block"]:
    m = unreal.load_asset(f"/Game/RR/Player/{m_name}")
    if not m:
        continue
    out[m_name] = {
        "length": m.get_play_length(),
        "num_sections": m.get_num_sections(),
        "section_names": [str(m.get_section_name(i)) for i in range(m.get_num_sections())]
    }
    # Check notifies
    notifies = []
    for n in m.notifies:
        cname = n.notify_state_class.get_class().get_name() if n.notify_state_class else (n.notify.get_class().get_name() if n.notify else "None")
        notifies.append({
            "time": n.get_editor_property("display_time") if hasattr(n, "display_time") else 0.0,
            "class": cname
        })
    out[m_name]["notifies"] = notifies

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_ual_montages.json", "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_UAL_MONTAGES_DONE")
