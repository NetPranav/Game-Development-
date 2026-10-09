import unreal, json

eal = unreal.EditorAssetLibrary
AL = unreal.AnimationLibrary

rtg = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/RTG_Pro_to_UEFN")
rc = unreal.IKRetargeterController.get_controller(rtg)

# Unmap target Root chain (which was mistakenly mapped to RightFoot)
rc.set_source_chain("Root", "")

# Also unmap toe and metacarpal if they map to whole foot or whole finger
for c_name in ["RightToe", "LeftToe", "LeftIndexMetacarpal", "LeftMiddleMetacarpal", "LeftRingMetacarpal", "LeftPinkyMetacarpal", "RightIndexMetacarpal", "RightMiddleMetacarpal", "RightRingMetacarpal", "RightPinkyMetacarpal"]:
    rc.set_source_chain(c_name, "")

eal.save_loaded_asset(rtg)
print("Updated RTG_Pro_to_UEFN mappings!")

# Test retargeting Pro_Slash_1 to check
ar = unreal.AssetRegistryHelpers.get_asset_registry()
a_data = ar.get_asset_by_object_path("/Game/RR/ExternalAnims/ProSwordShield/Pro_Slash_1.Pro_Slash_1")

# Remove old RT_Pro_Slash_1 to retarget fresh
eal.delete_asset("/Game/RR/Player/Anims/RT_Pro_Slash_1")

batch = unreal.IKRetargetBatchOperation()
batch.duplicate_and_retarget(
    [a_data],
    rtg.get_editor_property("source_ik_rig"),
    rtg.get_editor_property("target_ik_rig"),
    rtg,
    "/Game/RR/Player/Anims",
    "RT_",
    "",
    True
)

seq = unreal.load_asset("/Game/RR/Player/Anims/RT_Pro_Slash_1")
out = {}
if seq:
    for b in ["root", "pelvis", "spine_01", "spine_03", "thigh_l", "thigh_r"]:
        t0 = AL.get_bone_pose_for_frame(seq, b, 0, False)
        out[b] = {
            "loc": [t0.translation.x, t0.translation.y, t0.translation.z],
            "rot": [t0.rotation.rotator().pitch, t0.rotation.rotator().yaw, t0.rotation.rotator().roll]
        }

with open(unreal.Paths.project_saved_dir() + "RRTools/test_fixed_slash1.json", "w") as f:
    json.dump(out, f, indent=2)

print("FIX_RTG_AND_TEST_DONE")
