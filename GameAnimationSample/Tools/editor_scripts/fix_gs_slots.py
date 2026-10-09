import unreal

print("=== FIXING GREYSTONE MONTAGE SLOTS ===")

montages = [
    "/Game/RR/Greystone/AM_GS_HitReact_Front",
    "/Game/RR/Greystone/AM_GS_HitReact_Back",
    "/Game/RR/Greystone/AM_GS_HitReact_Left",
    "/Game/RR/Greystone/AM_GS_HitReact_Right",
    "/Game/RR/Greystone/AM_GS_Stun",
    "/Game/RR/Greystone/AM_GS_Death",
    "/Game/RR/Greystone/AM_GS_Deflect",
    "/Game/RR/Greystone/AM_GS_Heavy",
]

for m_path in montages:
    m = unreal.load_asset(m_path)
    if not m:
        print(f"FAILED TO LOAD: {m_path}")
        continue
    tracks = m.get_editor_property("slot_anim_tracks")
    changed = False
    for t in tracks:
        current_slot = str(t.get_editor_property("slot_name"))
        if current_slot != "UpperBody":
            t.set_editor_property("slot_name", "UpperBody")
            changed = True
            print(f"[{m.get_name()}] Changed slot track from '{current_slot}' to 'UpperBody'")
    if changed:
        m.set_editor_property("slot_anim_tracks", tracks)
        unreal.EditorAssetLibrary.save_loaded_asset(m)
        print(f"[{m.get_name()}] Saved asset successfully")
    else:
        print(f"[{m.get_name()}] Already has UpperBody slot")

# Now check and update BP_RRBoss
boss_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RRBoss")
if boss_bp:
    cdo = unreal.get_default_object(boss_bp.generated_class())
    if hasattr(cdo, "anim_slot"):
        cdo.set_editor_property("anim_slot", "UpperBody")
        print("Updated BP_RRBoss CDO anim_slot to UpperBody")
    
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sub.k2_gather_subobject_data_for_blueprint(boss_bp)
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if o and "CombatComponent" in o.get_class().get_name():
            o.set_editor_property("montage_slot_override", "UpperBody")
            print("Updated BP_RRBoss CombatComponent montage_slot_override to UpperBody")
    
    unreal.EditorAssetLibrary.save_loaded_asset(boss_bp)
    print("Saved BP_RRBoss")

# Also check BP_RREnemy_Greystone if it exists
enemy_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RREnemy_Greystone")
if enemy_bp:
    cdo = unreal.get_default_object(enemy_bp.generated_class())
    if hasattr(cdo, "anim_slot"):
        cdo.set_editor_property("anim_slot", "UpperBody")
        print("Updated BP_RREnemy_Greystone CDO anim_slot to UpperBody")
    for h in sub.k2_gather_subobject_data_for_blueprint(enemy_bp):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if o and "CombatComponent" in o.get_class().get_name():
            o.set_editor_property("montage_slot_override", "UpperBody")
            print("Updated BP_RREnemy_Greystone CombatComponent montage_slot_override to UpperBody")
    unreal.EditorAssetLibrary.save_loaded_asset(enemy_bp)
    print("Saved BP_RREnemy_Greystone")

print("=== FIX_GS_SLOTS_COMPLETED ===")
