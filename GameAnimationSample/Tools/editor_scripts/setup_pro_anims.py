import unreal, os, json

WS = "A:/UNREAL PROJECT/Game/GameAnimationSample"
anim_src_dir = f"{WS}/ExternalAnims/ProSwordShield"
anim_dst_dir = "/Game/RR/ExternalAnims/ProSwordShield"
player_anims_dst = "/Game/RR/Player/Anims"
player_montages_dst = "/Game/RR/Player"

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()

out = {"status": "started"}

# 1. Check / Load base skeleton
skel_path = f"{anim_dst_dir}/SKM_ProSwordShield_Base_Skeleton"
skeleton = unreal.load_asset(skel_path)
skel_mesh = unreal.load_asset(f"{anim_dst_dir}/SKM_ProSwordShield_Base")

if not skeleton or not skel_mesh:
    out["error"] = "Base mesh or skeleton not found"
    print("ERROR:", out["error"])
else:
    print(f"Loaded base skeleton: {skeleton.get_name()}")
    
    # 2. Import key FBX animations
    key_anims = [
        ("sword and shield slash.fbx", "sword_and_shield_slash"),
        ("sword and shield slash (2).fbx", "sword_and_shield_slash_2"),
        ("sword and shield slash (3).fbx", "sword_and_shield_slash_3"),
        ("sword and shield slash (4).fbx", "sword_and_shield_slash_4"),
        ("sword and shield slash (5).fbx", "sword_and_shield_slash_5"),
        ("sword and shield attack.fbx", "sword_and_shield_attack"),
        ("sword and shield attack (2).fbx", "sword_and_shield_attack_2"),
        ("sword and shield attack (3).fbx", "sword_and_shield_attack_3"),
        ("sword and shield kick.fbx", "sword_and_shield_kick"),
        ("sword and shield power up.fbx", "sword_and_shield_power_up"),
        ("sword and shield block.fbx", "sword_and_shield_block"),
        ("sword and shield impact.fbx", "sword_and_shield_impact"),
        ("draw sword 1.fbx", "draw_sword_1"),
        ("sheath sword 1.fbx", "sheath_sword_1"),
    ]
    
    imported = []
    import_tasks = []
    for fbx_name, asset_name in key_anims:
        dest_pkg = f"{anim_dst_dir}/{asset_name}"
        fbx_file = f"{anim_src_dir}/{fbx_name}"
        if not os.path.exists(fbx_file):
            print(f"File not found: {fbx_file}")
            continue
        if not eal.does_asset_exist(dest_pkg):
            t = unreal.AssetImportTask()
            t.filename = fbx_file
            t.destination_path = anim_dst_dir
            t.destination_name = asset_name
            t.automated = True
            t.save = True
            t.replace_existing = True
            
            ui = unreal.FbxImportUI()
            ui.import_mesh = False
            ui.import_animations = True
            ui.skeleton = skeleton
            t.options = ui
            import_tasks.append(t)
        else:
            imported.append(asset_name)
            
    if import_tasks:
        print(f"Importing {len(import_tasks)} FBX animations...")
        at.import_asset_tasks(import_tasks)
        for t in import_tasks:
            imported.extend(t.imported_object_paths)
            
    out["imported_fbx"] = imported
    print(f"Total source anims ready: {len(imported)}")

    # 3. Check RTG
    rtg_path = f"{anim_dst_dir}/RTG_Pro_to_UEFN"
    rtg = unreal.load_asset(rtg_path)
    ue_mesh = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
    ue_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")
    rig = unreal.load_asset(f"{anim_dst_dir}/IK_ProSwordShield")

    if not rtg:
        rtg = at.create_asset("RTG_Pro_to_UEFN", anim_dst_dir, unreal.IKRetargeter, unreal.IKRetargetFactory())
    
    if rtg and rig and ue_rig:
        c = unreal.IKRetargeterController.get_controller(rtg)
        S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
        c.set_ik_rig(S, rig)
        c.set_ik_rig(T, ue_rig)
        c.set_preview_mesh(S, skel_mesh)
        c.set_preview_mesh(T, ue_mesh)
        if c.get_num_retarget_ops() == 0:
            c.add_default_ops()
        c.assign_ik_rig_to_all_ops(S, rig)
        c.assign_ik_rig_to_all_ops(T, ue_rig)
        c.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
        eal.save_asset(rtg_path)
        print("RTG_Pro_to_UEFN updated and saved!")

    # 4. Retarget animations to player
    anims_to_retarget = []
    for _, asset_name in key_anims:
        src = unreal.load_asset(f"{anim_dst_dir}/{asset_name}")
        out_path = f"{player_anims_dst}/RT_Pro_{asset_name}"
        if src and not eal.does_asset_exist(out_path):
            anims_to_retarget.append(src)

    if anims_to_retarget and rtg:
        print(f"Retargeting {len(anims_to_retarget)} animations to player...")
        retargeted = unreal.IKRetargetBatchOperation().duplicate_and_retarget(
            anims_to_retarget, skel_mesh, ue_mesh, rtg, "", "", "RT_Pro_", "", player_anims_dst, False, False, True
        )
        for a in retargeted:
            eal.save_asset(str(a.package_name))
        print(f"Retargeted {len(retargeted)} player combat animations!")
        out["retargeted_count"] = len(retargeted)

    # 5. Create AnimMontages
    def mk_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0):
        path = f"{player_montages_dst}/{montage_name}"
        seq = unreal.load_asset(f"{player_anims_dst}/{seq_name}")
        if not seq:
            print(f"Warning: Sequence {seq_name} not found for montage {montage_name}")
            return None
        if eal.does_asset_exist(path):
            m = unreal.load_asset(path)
        else:
            f = unreal.AnimMontageFactory()
            f.set_editor_property("source_animation", seq)
            m = at.create_asset(montage_name, player_montages_dst, unreal.AnimMontage, f)
        if m:
            b = m.get_editor_property("blend_in")
            b.set_editor_property("blend_time", bi)
            m.set_editor_property("blend_in", b)
            b = m.get_editor_property("blend_out")
            b.set_editor_property("blend_time", bo)
            m.set_editor_property("blend_out", b)
            m.set_editor_property("rate_scale", rate)
            eal.save_loaded_asset(m)
            print(f"Montage {montage_name} ready!")
        return m

    # Light combos (5-step progressive chain)
    l1 = mk_montage("RT_Pro_sword_and_shield_slash", "AM_P_Pro_L1", 0.05, 0.15, 1.1)
    l2 = mk_montage("RT_Pro_sword_and_shield_slash_2", "AM_P_Pro_L2", 0.05, 0.15, 1.15)
    l3 = mk_montage("RT_Pro_sword_and_shield_slash_3", "AM_P_Pro_L3", 0.05, 0.15, 1.2)
    l4 = mk_montage("RT_Pro_sword_and_shield_slash_4", "AM_P_Pro_L4", 0.05, 0.15, 1.25)
    l5 = mk_montage("RT_Pro_sword_and_shield_slash_5", "AM_P_Pro_L5", 0.05, 0.25, 1.3)

    # Heavy combos
    h1 = mk_montage("RT_Pro_sword_and_shield_attack", "AM_P_Pro_H1", 0.08, 0.2, 1.0)
    h2 = mk_montage("RT_Pro_sword_and_shield_attack_2", "AM_P_Pro_H2", 0.08, 0.2, 1.05)
    h3 = mk_montage("RT_Pro_sword_and_shield_kick", "AM_P_Pro_Kick", 0.08, 0.2, 1.1)
    h4 = mk_montage("RT_Pro_sword_and_shield_power_up", "AM_P_Pro_PowerUp", 0.1, 0.25, 1.0)

    # Block & impact
    blk = mk_montage("RT_Pro_sword_and_shield_block", "AM_P_Pro_Block", 0.05, 0.1, 1.0)
    imp = mk_montage("RT_Pro_sword_and_shield_impact", "AM_P_Pro_Impact", 0.05, 0.1, 1.0)

    # Equip / Unequip
    eq = mk_montage("RT_Pro_draw_sword_1", "AM_P_Pro_Equip", 0.08, 0.15, 1.2)
    uneq = mk_montage("RT_Pro_sheath_sword_1", "AM_P_Pro_Unequip", 0.08, 0.15, 1.2)

    out["montages_created"] = {
        "light": [m.get_name() for m in [l1, l2, l3, l4, l5] if m],
        "heavy": [m.get_name() for m in [h1, h2, h3, h4] if m],
        "block": blk.get_name() if blk else None,
        "impact": imp.get_name() if imp else None,
        "equip": eq.get_name() if eq else None,
        "unequip": uneq.get_name() if uneq else None
    }

with open(f"{WS}/Saved/RRTools/setup_pro_anims.json", "w") as f:
    json.dump(out, f, indent=2)

print("SETUP_PRO_ANIMS_DONE")
