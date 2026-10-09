import unreal, json, os

abp = unreal.load_asset("/Game/Blueprints/SandboxCharacter_Mover_ABP")
t3d_path = unreal.Paths.project_saved_dir() + "RRTools/Sandbox_ABP.t3d"

task = unreal.AssetExportTask()
task.object = abp
task.filename = t3d_path
task.automated = True
task.prompt = False
unreal.Exporter.run_asset_export_task(task)

found = []
if os.path.exists(t3d_path):
    with open(t3d_path, "r", errors="ignore") as f:
        for line in f:
            if "Slot" in line or "slot" in line:
                found.append(line.strip())

with open(unreal.Paths.project_saved_dir() + "RRTools/abp_slots_found.json", "w") as f:
    json.dump(found, f, indent=2)

print(f"Export done, found {len(found)} slot lines")
