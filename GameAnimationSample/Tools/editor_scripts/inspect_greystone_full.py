import unreal, json, os

out = {}

# 1. Export Greystone_AnimBlueprint to t3d
abp_path = "/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint"
abp = unreal.load_asset(abp_path)

if abp:
    out["abp_name"] = abp.get_name()
    t3d_path = unreal.Paths.project_saved_dir() + "RRTools/Greystone_AnimBlueprint.t3d"
    task = unreal.AssetExportTask()
    task.object = abp
    task.filename = t3d_path
    task.selected = False
    task.replace_identical = True
    task.prompt = False
    task.automated = True
    export_ok = unreal.Exporter.run_asset_export_task(task)
    out["t3d_export_ok"] = export_ok
    out["t3d_path"] = t3d_path

# 2. Check Hit reaction montages
hr_names = ["AM_GS_HitReact_Front", "AM_GS_HitReact_Back", "AM_GS_HitReact_Left", "AM_GS_HitReact_Right", "AM_GS_Stun", "AM_GS_Death"]
montages_info = {}
for m_name in hr_names:
    m = unreal.load_asset(f"/Game/RR/Greystone/{m_name}")
    if m:
        m_tracks = []
        for track in m.get_editor_property("slot_anim_tracks"):
            slot_n = str(track.get_editor_property("slot_name"))
            anim_segs = []
            anim_track = track.get_editor_property("anim_track")
            for seg in anim_track.get_editor_property("anim_segments"):
                anim_ref = seg.get_editor_property("anim_reference")
                anim_segs.append(str(anim_ref.get_name()) if anim_ref else "None")
            m_tracks.append({"slot": slot_n, "segments": anim_segs})
        montages_info[m_name] = {
            "skeleton": str(m.get_editor_property("skeleton").get_name()) if m.get_editor_property("skeleton") else "None",
            "tracks": m_tracks,
            "rate_scale": m.get_editor_property("rate_scale"),
            "sequence_length": m.get_editor_property("sequence_length"),
        }
    else:
        montages_info[m_name] = "NOT_FOUND"
out["hit_montages"] = montages_info

# 3. Check Greystone animations folder for walk, jog, run animations
ar = unreal.AssetRegistryHelpers.get_asset_registry()
anim_assets = ar.get_assets(unreal.ARFilter(
    package_paths=["/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations"],
    recursive_paths=True
))
out["all_greystone_anims"] = [str(a.asset_name) for a in anim_assets]

save_file = unreal.Paths.project_saved_dir() + "RRTools/inspect_greystone_full.json"
with open(save_file, "w") as f:
    json.dump(out, f, indent=2, default=str)

print("INSPECT_GREYSTONE_FULL_DONE")
