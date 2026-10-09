import unreal, os, json

eal = unreal.EditorAssetLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
AL = unreal.AnimationLibrary

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

# 1. Ensure Root chain is UNMAPPED
rc.set_source_chain("Root", "")

for c_name in [
    "RightToe", "LeftToe", 
    "LeftIndexMetacarpal", "LeftMiddleMetacarpal", "LeftRingMetacarpal", "LeftPinkyMetacarpal", 
    "RightIndexMetacarpal", "RightMiddleMetacarpal", "RightRingMetacarpal", "RightPinkyMetacarpal"
]:
    try:
        rc.set_source_chain(c_name, "")
    except Exception:
        pass

eal.save_loaded_asset(rtg)
print("Saved clean RTG_Pro_to_UEFN mappings!")

# 2. Retarget to /Game/RR/Player/Anims/Pro
skel_mesh = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
target_folder = "/Game/RR/Player/Anims/Pro"

src_anims = [
    "Pro_Slash_1",
    "Pro_Slash_3",
    "Pro_Slash_5",
    "Pro_Block",
    "Pro_Attack_2",
    "Pro_Impact",
    "Pro_Kick",
    "Pro_PowerUp",
    "Pro_Draw",
    "Pro_Sheath"
]

asset_datas = []
for name in src_anims:
    path = f"/Game/RR/ExternalAnims/ProSwordShield/{name}.{name}"
    data = ar.get_asset_by_object_path(path)
    if data and data.is_valid():
        asset_datas.append(data)
    else:
        print(f"Warning: could not find AssetData for {path}")

print(f"Found {len(asset_datas)} AssetData items to retarget")

batch = unreal.IKRetargetBatchOperation()
retargeted = batch.duplicate_and_retarget(
    asset_datas, skel_mesh, ue_mesh, rtg, "", "", "RT_", "", target_folder, False, False, True
)

print(f"Batch retarget produced {len(retargeted)} assets!")
for a in retargeted:
    eal.save_asset(str(a.package_name))

# 3. Verify bone poses on retargeted slashes
check_results = {}
for name in ["RT_Pro_Slash_1", "RT_Pro_Slash_3", "RT_Pro_Slash_5", "RT_Pro_Block"]:
    seq = unreal.load_asset(f"{target_folder}/{name}")
    if seq:
        t0_root = AL.get_bone_pose_for_frame(seq, "root", 0, False)
        t0_pelvis = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
        check_results[name] = {
            "root_loc": [round(t0_root.translation.x, 2), round(t0_root.translation.y, 2), round(t0_root.translation.z, 2)],
            "root_rot": [round(t0_root.rotation.rotator().pitch, 2), round(t0_root.rotation.rotator().yaw, 2), round(t0_root.rotation.rotator().roll, 2)],
            "pelvis_loc": [round(t0_pelvis.translation.x, 2), round(t0_pelvis.translation.y, 2), round(t0_pelvis.translation.z, 2)],
            "pelvis_rot": [round(t0_pelvis.rotation.rotator().pitch, 2), round(t0_pelvis.rotation.rotator().yaw, 2), round(t0_pelvis.rotation.rotator().roll, 2)]
        }
    else:
        check_results[name] = "NOT FOUND"

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/batch_retarget_check.json", "w") as f:
    json.dump(check_results, f, indent=2)

print("BATCH_RETARGET_TEST_COMPLETE")
