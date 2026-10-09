import unreal, json

da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
out = {}

for prop in ["animations", "visuals", "combat", "abilities"]:
    try:
        val = da.get_editor_property(prop)
        props_dict = {}
        for sub_name in ["idle_animation", "idle_montage", "light_attack_combos", "heavy_attack_combos", "block_montage", "deflect_montage", "holster_montage", "draw_montage", "hand_offset", "attachment_socket", "holster_offset", "holster_socket", "static_mesh"]:
            try:
                props_dict[sub_name] = str(val.get_editor_property(sub_name))
            except Exception:
                pass
        out[prop] = props_dict
    except Exception as e:
        out[prop] = str(e)

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_da_full.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_DA_FULL_DONE")
