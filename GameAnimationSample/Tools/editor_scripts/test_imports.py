import unreal
import os

print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()

# Test 1: Import Colosseum C.fbx as StaticMesh into /Game/RR/Environment/Colosseum
fbx_path = "A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalModels/Colosseum/source/C.fbx"
col_dst = "/Game/RR/Environment/Colosseum"

if not eal.does_asset_exist(col_dst + "/SM_Colosseum"):
    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = col_dst
    task.destination_name = "SM_Colosseum"
    task.automated = True
    task.save = True
    task.replace_existing = True
    
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.generate_lightmap_u_vs = True
    task.options = options
    
    at.import_asset_tasks([task])
    print("Colosseum imported paths:", task.imported_object_paths)
else:
    print("SM_Colosseum already exists!")

# Test 2: Try importing GLB files
for name, glb_file in [("SM_LongSword", "long_sword.glb"), ("SM_VikingShield", "viking_shield.glb")]:
    glb_path = f"A:/UNREAL PROJECT/Game/GameAnimationSample/ExternalModels/Fab/{glb_file}"
    dst_path = f"/Game/RR/Weapons/Fab"
    
    task = unreal.AssetImportTask()
    task.filename = glb_path
    task.destination_path = dst_path
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    
    at.import_asset_tasks([task])
    print(f"GLB import result for {name}:", task.imported_object_paths)

print("TEST_IMPORTS_COMPLETE")
