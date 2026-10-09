import unreal, json

eal = unreal.EditorAssetLibrary
AL = unreal.AnimationLibrary

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

# Unmap target Root chain
rc.set_source_chain("Root", "")

for c_name in [
    "RightToe", "LeftToe", 
    "LeftIndexMetacarpal", "LeftMiddleMetacarpal", "LeftRingMetacarpal", "LeftPinkyMetacarpal", 
    "RightIndexMetacarpal", "RightMiddleMetacarpal", "RightRingMetacarpal", "RightPinkyMetacarpal"
]:
    rc.set_source_chain(c_name, "")

eal.save_loaded_asset(rtg)
print("Updated RTG_Pro_to_UEFN mappings!")

# Retarget Pro_Slash_1
skel_mesh = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
player_anims_dst = "/Game/RR/Player/Anims"

ar = unreal.AssetRegistryHelpers.get_asset_registry()
a_data = ar.get_asset_by_object_path("/Game/RR/ExternalAnims/ProSwordShield/Pro_Slash_1.Pro_Slash_1")

batch = unreal.IKRetargetBatchOperation()
retargeted = batch.duplicate_and_retarget(
    [a_data], skel_mesh, ue_mesh, rtg, "", "", "RT_", "", player_anims_dst, False, False, True
)
for a in retargeted:
    eal.save_asset(str(a.package_name))

seq = unreal.load_asset("/Game/RR/Player/Anims/RT_Pro_Slash_1")
out = {}
if seq:
    for b in ["root", "pelvis", "spine_01", "spine_02", "spine_03", "thigh_l", "thigh_r"]:
        t0 = AL.get_bone_pose_for_frame(seq, b, 0, False)
        out[b] = {
            "loc": [t0.translation.x, t0.translation.y, t0.translation.z],
            "rot": [t0.rotation.rotator().pitch, t0.rotation.rotator().yaw, t0.rotation.rotator().roll]
        }

with open(unreal.Paths.project_saved_dir() + "RRTools/clean_retarget_slash1.json", "w") as f:
    json.dump(out, f, indent=2)

print("TEST_RETARGET_CLEAN_DONE")
