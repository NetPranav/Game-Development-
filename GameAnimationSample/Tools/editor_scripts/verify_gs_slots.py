import unreal, json

out = {}

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

out["montages"] = {}
for m_path in montages:
    m = unreal.load_asset(m_path)
    if m:
        tracks = m.get_editor_property("slot_anim_tracks")
        slots = [str(t.get_editor_property("slot_name")) for t in tracks]
        out["montages"][m.get_name()] = slots

boss_bp = unreal.load_asset("/Game/RR/Blueprints/BP_RRBoss")
if boss_bp:
    cdo = unreal.get_default_object(boss_bp.generated_class())
    out["boss_cdo_anim_slot"] = str(getattr(cdo, "anim_slot", "None"))
    
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sub.k2_gather_subobject_data_for_blueprint(boss_bp)
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if o and "CombatComponent" in o.get_class().get_name():
            out["boss_combat_slot_override"] = str(o.get_editor_property("montage_slot_override"))

with open(unreal.Paths.project_saved_dir() + "RRTools/verify_gs_slots.json", "w") as f:
    json.dump(out, f, indent=2)

print("VERIFY_GS_SLOTS_DONE")
