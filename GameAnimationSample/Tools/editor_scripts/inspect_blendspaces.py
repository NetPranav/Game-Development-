import unreal, json

out = {}
bs_paths = [
    "/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Blendspaces/Locomotion_BS",
    "/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Blendspaces/Greystone_Locomotion_BS",
    "/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Blendspaces/JogFwdSlopeLean"
]

for p in bs_paths:
    bs = unreal.load_asset(p)
    if bs:
        samples = []
        try:
            for s in bs.get_editor_property("sample_data"):
                anim = s.get_editor_property("animation")
                samples.append({
                    "anim": str(anim.get_name()) if anim else "None",
                    "sample_value": str(s.get_editor_property("sample_value"))
                })
        except Exception as e:
            samples = f"err: {e}"
        out[p.split("/")[-1]] = {
            "class": bs.get_class().get_name(),
            "samples": samples
        }
    else:
        out[p.split("/")[-1]] = "NOT_FOUND"

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_blendspaces.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BLENDSPACES_DONE")
