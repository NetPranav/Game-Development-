import unreal, json

eal = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()

anims_folder = "/Game/RR/Player/Anims/Pro"
montages_folder = "/Game/RR/Player/Pro"

def create_or_update_montage(seq_name, montage_name, bi=0.06, bo=0.15, rate=1.0):
    seq_path = f"{anims_folder}/{seq_name}"
    seq = unreal.load_asset(seq_path)
    if not seq:
        print(f"Error: Sequence not found: {seq_path}")
        return None
    
    m_path = f"{montages_folder}/{montage_name}"
    m = unreal.load_asset(m_path)
    if not m:
        f = unreal.AnimMontageFactory()
        f.set_editor_property("source_animation", seq)
        m = at.create_asset(montage_name, montages_folder, unreal.AnimMontage, f)
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
        print(f"Montage ready: {montage_name}")
    return m

# 3 Grounded Slashes:
l1 = create_or_update_montage("RT_Pro_Slash_1", "AM_P_Pro_L1", bi=0.05, bo=0.15, rate=1.15)
l2 = create_or_update_montage("RT_Pro_Slash_3", "AM_P_Pro_L2", bi=0.05, bo=0.15, rate=1.20)
l3 = create_or_update_montage("RT_Pro_Slash_5", "AM_P_Pro_L3", bi=0.05, bo=0.25, rate=1.25)

# Block & Impact
blk = create_or_update_montage("RT_Pro_Block", "AM_P_Pro_Block", bi=0.08, bo=0.15, rate=1.0)
imp = create_or_update_montage("RT_Pro_Impact", "AM_P_Pro_Impact", bi=0.05, bo=0.15, rate=1.0)

# Heavy & utilities
h1 = create_or_update_montage("RT_Pro_Attack_2", "AM_P_Pro_H1", bi=0.08, bo=0.20, rate=1.1)
kick = create_or_update_montage("RT_Pro_Kick", "AM_P_Pro_Kick", bi=0.08, bo=0.20, rate=1.1)
pup = create_or_update_montage("RT_Pro_PowerUp", "AM_P_Pro_PowerUp", bi=0.10, bo=0.25, rate=1.0)
eq = create_or_update_montage("RT_Pro_Draw", "AM_P_Pro_Equip", bi=0.08, bo=0.15, rate=1.2)
uneq = create_or_update_montage("RT_Pro_Sheath", "AM_P_Pro_Unequip", bi=0.08, bo=0.15, rate=1.2)

# Also update the legacy path /Game/RR/Player/ if writable
for m_obj, old_name in [(l1, "AM_P_Pro_L1"), (l2, "AM_P_Pro_L2"), (l3, "AM_P_Pro_L3"), (blk, "AM_P_Pro_Block")]:
    old_path = f"/Game/RR/Player/{old_name}"
    old_m = unreal.load_asset(old_path)
    if old_m and m_obj:
        try:
            tracks = old_m.get_editor_property("slot_anim_tracks")
            new_tracks = m_obj.get_editor_property("slot_anim_tracks")
            if tracks and new_tracks:
                tracks[0].get_editor_property("anim_track").get_editor_property("anim_segments")[0].set_editor_property(
                    "anim_reference",
                    new_tracks[0].get_editor_property("anim_track").get_editor_property("anim_segments")[0].get_editor_property("anim_reference")
                )
            old_m.set_editor_property("rate_scale", m_obj.get_editor_property("rate_scale"))
            eal.save_loaded_asset(old_m)
            print(f"Updated legacy montage: {old_name}")
        except Exception as e:
            print(f"Could not update legacy {old_name}: {e}")

# Update DA_PlayerWeapon
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
    print("Updated DA_PlayerWeapon successfully!")

out = {
    "light": [l1.get_path_name() if l1 else None, l2.get_path_name() if l2 else None, l3.get_path_name() if l3 else None],
    "block": blk.get_path_name() if blk else None,
    "da_updated": True
}
with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/montages_setup.json", "w") as f:
    json.dump(out, f, indent=2)

print("SETUP_PRO_MONTAGES_DONE")
