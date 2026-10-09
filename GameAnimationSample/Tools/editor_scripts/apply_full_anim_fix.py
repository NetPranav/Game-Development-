import unreal, os, json

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
ar = unreal.AssetRegistryHelpers.get_asset_registry()
AL = unreal.AnimationLibrary

anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"
player_montages_dst = "/Game/RR/Player"

# 1. Update RTG_Pro_to_UEFN
rtg_path = f"{anim_dst_dir}/RTG_Pro_to_UEFN"
rtg = unreal.load_asset(rtg_path)
rc = unreal.IKRetargeterController.get_controller(rtg)

# Explicitly UNMAP Root chain so it does NOT follow RightFoot!
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

# Ensure standard chains are mapped
chain_maps = {
    "Spine": "Spine",
    "Neck": "Neck",
    "Head": "Head",
    "LeftClavicle": "LeftClavicle",
    "LeftArm": "LeftArm",
    "LeftLeg": "LeftLeg",
    "RightClavicle": "RightClavicle",
    "RightArm": "RightArm",
    "RightLeg": "RightLeg",
    "LeftThumb": "LeftThumb",
    "LeftIndex": "LeftIndex",
    "LeftMiddle": "LeftMiddle",
    "LeftRing": "LeftRing",
    "LeftPinky": "LeftPinky",
    "RightThumb": "RightThumb",
    "RightIndex": "RightIndex",
    "RightMiddle": "RightMiddle",
    "RightRing": "RightRing",
    "RightPinky": "RightPinky"
}
for tgt_c, src_c in chain_maps.items():
    try:
        rc.set_source_chain(tgt_c, src_c)
    except Exception as e:
        print(f"Failed mapping {tgt_c}: {e}")

eal.save_loaded_asset(rtg)
print("1. Saved clean RTG_Pro_to_UEFN mappings!")

# 2. Retarget Clean Animations
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")

# Grounded slashes:
# Pro_Slash_1 (right slash, delta Z = 11.8cm) -> RT_Pro_Slash_1
# Pro_Slash_3 (left slash, delta Z = 13.8cm) -> RT_Pro_Slash_2
# Pro_Slash_5 (overhead finisher, delta Z = 14.4cm) -> RT_Pro_Slash_3
anims_to_retarget = [
    ("Pro_Slash_1", "RT_Pro_Slash_1"),
    ("Pro_Slash_3", "RT_Pro_Slash_2"),
    ("Pro_Slash_5", "RT_Pro_Slash_3"),
    ("Pro_Block", "RT_Pro_Block"),
    ("Pro_Attack_2", "RT_Pro_H1"),
    ("Pro_Impact", "RT_Pro_Impact"),
    ("Pro_Kick", "RT_Pro_Kick"),
    ("Pro_PowerUp", "RT_Pro_PowerUp"),
    ("Pro_Draw", "RT_Pro_Equip"),
    ("Pro_Sheath", "RT_Pro_Unequip")
]

batch = unreal.IKRetargetBatchOperation()
retarget_log = {}

for src_name, tgt_name in anims_to_retarget:
    src_path = f"{anim_dst_dir}/{src_name}.{src_name}"
    data = ar.get_asset_by_object_path(src_path)
    if not data or not data.is_valid():
        print(f"Warning: AssetData not found for {src_path}")
        continue
    
    tgt_path = f"{player_anims_dst}/{tgt_name}"
    if eal.does_asset_exist(tgt_path):
        eal.delete_asset(tgt_path)

    retargeted = batch.duplicate_and_retarget(
        [data], skel_mesh, ue_mesh, rtg, "", "", "RT_", "", player_anims_dst, False, False, True
    )
    if retargeted:
        first = retargeted[0]
        cur_pkg = str(first.package_name)
        if cur_pkg != tgt_path:
            eal.rename_asset(cur_pkg, tgt_path)
            eal.save_asset(tgt_path)
        else:
            eal.save_asset(cur_pkg)
        retarget_log[tgt_name] = "Success"
        print(f"Retargeted {src_name} -> {tgt_name}")
    else:
        retarget_log[tgt_name] = "Failed"

# 3. Create / Update Montages
def setup_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0):
    seq_path = f"{player_anims_dst}/{seq_name}"
    seq = unreal.load_asset(seq_path)
    if not seq:
        print(f"Sequence not found: {seq_path}")
        return None
    
    m_path = f"{player_montages_dst}/{montage_name}"
    m = unreal.load_asset(m_path)
    if not m:
        f = unreal.AnimMontageFactory()
        f.set_editor_property("source_animation", seq)
        m = at.create_asset(montage_name, player_montages_dst, unreal.AnimMontage, f)
    else:
        tracks = m.get_editor_property("slot_anim_tracks")
        if tracks:
            anim_track = tracks[0].get_editor_property("anim_track")
            segs = anim_track.get_editor_property("anim_segments")
            if segs:
                segs[0].set_editor_property("anim_reference", seq)
                anim_track.set_editor_property("anim_segments", segs)
                tracks[0].set_editor_property("anim_track", anim_track)
                m.set_editor_property("slot_anim_tracks", tracks)

    if m:
        b = m.get_editor_property("blend_in")
        b.set_editor_property("blend_time", bi)
        m.set_editor_property("blend_in", b)
        
        b = m.get_editor_property("blend_out")
        b.set_editor_property("blend_time", bo)
        m.set_editor_property("blend_out", b)
        
        m.set_editor_property("rate_scale", rate)
        eal.save_loaded_asset(m)
        print(f"Saved montage: {montage_name}")
    return m

# Clean 3-hit grounded combo montages:
l1 = setup_montage("RT_Pro_Slash_1", "AM_P_Pro_L1", bi=0.05, bo=0.15, rate=1.15)
l2 = setup_montage("RT_Pro_Slash_2", "AM_P_Pro_L2", bi=0.05, bo=0.15, rate=1.20)
l3 = setup_montage("RT_Pro_Slash_3", "AM_P_Pro_L3", bi=0.05, bo=0.25, rate=1.25)

# Block & impact:
blk = setup_montage("RT_Pro_Block", "AM_P_Pro_Block", bi=0.08, bo=0.15, rate=1.0)
imp = setup_montage("RT_Pro_Impact", "AM_P_Pro_Impact", bi=0.05, bo=0.15, rate=1.0)

# Heavy & utilities:
h1 = setup_montage("RT_Pro_H1", "AM_P_Pro_H1", bi=0.08, bo=0.20, rate=1.1)
kick = setup_montage("RT_Pro_Kick", "AM_P_Pro_Kick", bi=0.08, bo=0.20, rate=1.1)
pup = setup_montage("RT_Pro_PowerUp", "AM_P_Pro_PowerUp", bi=0.10, bo=0.25, rate=1.0)
eq = setup_montage("RT_Pro_Equip", "AM_P_Pro_Equip", bi=0.08, bo=0.15, rate=1.2)
uneq = setup_montage("RT_Pro_Unequip", "AM_P_Pro_Unequip", bi=0.08, bo=0.15, rate=1.2)

# 4. Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    anims = da.get_editor_property("animations")
    anims.set_editor_property("light_attack_combos", [l1, l2, l3])
    anims.set_editor_property("heavy_attack_combos", [h1, kick, pup])
    anims.set_editor_property("block_montage", blk)
    anims.set_editor_property("parry_montage", blk)
    anims.set_editor_property("equip_animation", eq)
    anims.set_editor_property("unequip_animation", uneq)
    da.set_editor_property("animations", anims)
    eal.save_loaded_asset(da)
    print("4. Saved DA_PlayerWeapon!")

# 5. Verify bone poses
verify = {}
for anim_name in ["RT_Pro_Slash_1", "RT_Pro_Slash_2", "RT_Pro_Slash_3", "RT_Pro_Block"]:
    seq = unreal.load_asset(f"{player_anims_dst}/{anim_name}")
    if seq:
        t0_root = AL.get_bone_pose_for_frame(seq, "root", 0, False)
        t0_pelvis = AL.get_bone_pose_for_frame(seq, "pelvis", 0, False)
        verify[anim_name] = {
            "root_loc": [round(t0_root.translation.x, 2), round(t0_root.translation.y, 2), round(t0_root.translation.z, 2)],
            "root_rot": [round(t0_root.rotation.rotator().pitch, 2), round(t0_root.rotation.rotator().yaw, 2), round(t0_root.rotation.rotator().roll, 2)],
            "pelvis_loc": [round(t0_pelvis.translation.x, 2), round(t0_pelvis.translation.y, 2), round(t0_pelvis.translation.z, 2)],
            "pelvis_rot": [round(t0_pelvis.rotation.rotator().pitch, 2), round(t0_pelvis.rotation.rotator().yaw, 2), round(t0_pelvis.rotation.rotator().roll, 2)]
        }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/full_anim_fix_summary.json", "w") as f:
    json.dump({"retarget_log": retarget_log, "verify": verify}, f, indent=2)

print("APPLY_FULL_ANIM_FIX_COMPLETE")
