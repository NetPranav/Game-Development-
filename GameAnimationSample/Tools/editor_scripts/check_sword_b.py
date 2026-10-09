import json

# Check SM_Sword_B
b_obj_task = "SM_Sword_B"
import unreal

task = unreal.AssetExportTask()
task.object = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B")
task.filename = unreal.Paths.project_saved_dir() + "RRTools/SM_Sword_B.obj"
task.exporter = unreal.StaticMeshExporterOBJ()
task.automated = True
task.prompt = False
unreal.Exporter.run_asset_export_task(task)

verts = []
with open(task.filename, "r") as f:
    for line in f:
        if line.startswith("v "):
            parts = line.strip().split()
            verts.append((float(parts[1]), float(parts[2]), float(parts[3])))

ys = [v[1] for v in verts]
wide_verts = [v for v in verts if abs(v[0]) > 15.0]
wide_ys = [v[1] for v in wide_verts]

out = {
    "num_verts": len(verts),
    "y_range": [min(ys), max(ys)],
    "crossguard_y_range": [min(wide_ys), max(wide_ys)] if wide_ys else []
}

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/sword_b_check.json", "w") as f:
    json.dump(out, f, indent=2)

print("SWORD_B_CHECK_DONE")
