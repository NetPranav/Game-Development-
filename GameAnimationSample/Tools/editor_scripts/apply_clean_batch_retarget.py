import unreal, json

eal = unreal.EditorAssetLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()
at = unreal.AssetToolsHelpers.get_asset_tools()
AL = unreal.AnimationLibrary

anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"
player_montages_dst = "/Game/RR/Player"

rtg = unreal.load_asset(f"{anim_dst_dir}/RTG_Pro_to_UEFN")
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")
ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")

# 1. Retarget all animations
anims_to_retarget = [
    ("Pro_Slash_1", "RT_Pro_Slash_1"),
    ("Pro_Slash_3", "RT_Pro_Slash_2"),
    ("Pro_Attack_2", "RT_Pro_Slash_3"),
    ("Pro_Block", "RT_Pro_Block"),
    ("Pro_Impact", "RT_Pro_Impact"),
    ("Pro_Kick", "RT_Pro_Kick"),
    ("Pro_PowerUp", "RT_Pro_PowerUp"),
    ("Pro_Draw", "RT_Pro_Equip"),
    ("Pro_Sheath", "RT_Pro_Unequip")
]

batch = unreal.IKRetargetBatchOperation()
retarget_results = {}

for src_name, tgt_name in anims_to_retarget:
    tgt_path = f"{player_anims_dst}/{tgt_name}"
    if eal.does_asset_exist(tgt_path):
        eal.delete_asset(tgt_path)

    src_path = f"{anim_dst_dir}/{src_name}.{src_name}"
    data = ar.get_asset_by_object_path(src_path)
    if not data or not data.is_valid():
        retarget_results[tgt_name] = f"Source not found: {src_path}"
        continue

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
        retarget_results[tgt_name] = "Success"
    else:
        retarget_results[tgt_name] = "Failed"

# 2. Recreate Montages cleanly using AnimMontageFactory
def create_montage(seq_name, montage_name, bi=0.05, bo=0.15, rate=1.0):
    seq_path = f"{player_anims_dst}/{seq_name}"
    seq = unreal.load_asset(seq_path)
    if not seq:
        print(f"ERROR: Sequence {seq_path} not found!")
        return None

    m_path = f"{player_montages_dst}/{montage_name}"
    if eal.does_asset_exist(m_path):
        eal.delete_asset(m_path)

    factory = unreal.AnimMontageFactory()
    factory.set_editor_property("source_animation", seq)
    m = at.create_asset(montage_name, player_montages_dst, unreal.AnimMontage, factory)
    if m:
        b_in = m.get_editor_property("blend_in")
        b_in.set_editor_property("blend_time", bi)
        m.set_editor_property("blend_in", b_in)

        b_out = m.get_editor_property("blend_out")
        b_out.set_editor_property("blend_time", bo)
        m.set_editor_property("blend_out", b_out)

        m.set_editor_property("rate_scale", rate)
        eal.save_loaded_asset(m)
        print(f"Created clean montage: {montage_name} from {seq_name}")
    return m

l1 = create_montage("RT_Pro_Slash_1", "AM_P_Pro_L1", bi=0.05, bo=0.15, rate=1.15)
l2 = create_montage("RT_Pro_Slash_2", "AM_P_Pro_L2", bi=0.05, bo=0.15, rate=1.20)
l3 = create_montage("RT_Pro_Slash_3", "AM_P_Pro_L3", bi=0.05, bo=0.25, rate=1.25)
blk = create_montage("RT_Pro_Block", "AM_P_Pro_Block", bi=0.08, bo=0.15, rate=1.0)
imp = create_montage("RT_Pro_Impact", "AM_P_Pro_Impact", bi=0.05, bo=0.15, rate=1.0)
kick = create_montage("RT_Pro_Kick", "AM_P_Pro_Kick", bi=0.08, bo=0.20, rate=1.1)
pup = create_montage("RT_Pro_PowerUp", "AM_P_Pro_PowerUp", bi=0.10, bo=0.25, rate=1.0)
eq = create_montage("RT_Pro_Equip", "AM_P_Pro_Equip", bi=0.08, bo=0.15, rate=1.2)
uneq = create_montage("RT_Pro_Unequip", "AM_P_Pro_Unequip", bi=0.08, bo=0.15, rate=1.2)

# 3. Update DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    anims = da.get_editor_property("animations")
    anims.set_editor_property("light_attack_combos", [l1, l2, l3])
    anims.set_editor_property("heavy_attack_combos", [l3, kick, pup])
    anims.set_editor_property("block_montage", blk)
    anims.set_editor_property("parry_montage", blk)
    anims.set_editor_property("equip_animation", eq)
    anims.set_editor_property("unequip_animation", uneq)
    da.set_editor_property("animations", anims)
    eal.save_loaded_asset(da)
    print("Saved DA_PlayerWeapon with clean 3-hit combo and block!")

# 4. Verification of bone positions
bone_checks = {}
for anim_name in ["RT_Pro_Slash_1", "RT_Pro_Slash_2", "RT_Pro_Slash_3", "RT_Pro_Block"]:
    seq = unreal.load_asset(f"{player_anims_dst}/{anim_name}")
    if seq:
        num_f = AL.get_num_frames(seq)
        p_zs = []
        r_rots = []
        for f in range(num_f):
            p = AL.get_bone_pose_for_frame(seq, "pelvis", f, False)
            r = AL.get_bone_pose_for_frame(seq, "root", f, False)
            p_zs.append(p.translation.z)
            r_rots.append([r.rotation.rotator().pitch, r.rotation.rotator().yaw, r.rotation.rotator().roll])
        bone_checks[anim_name] = {
            "num_frames": num_f,
            "pelvis_z_min": min(p_zs),
            "pelvis_z_max": max(p_zs),
            "max_root_pitch": max(abs(rot[0]) for rot in r_rots)
        }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/final_retarget_summary.json", "w") as f:
    json.dump({"retarget": retarget_results, "bone_checks": bone_checks}, f, indent=2)

print("ALL_ANIM_FIXES_COMPLETE")
