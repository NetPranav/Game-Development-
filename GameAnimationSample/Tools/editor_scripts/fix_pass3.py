import unreal
# Asset half of the "sinking / too fast" pass (run after fix_ual2_pelvis.py, editor closed).
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
eal = unreal.EditorAssetLibrary

# 1) Robot display retarget back to the GASP defaults: the IK ops plant the robot's feet on uneven ground, and
#    ABP_GenericRetarget's UpdateRetargetProfile reads the IK-chain op every frame (it errored while they were disabled).
#    The old "sitting" pose came from the broken UAL2 pelvis tracks, not from these ops.
rtg = unreal.load_asset("/Game/Characters/UE4_Mannequin/Rigs/RTG_UEFN_to_UE4_Mannequin")
rc = unreal.IKRetargeterController.get_controller(rtg)
for i in range(rc.get_num_retarget_ops()):
    if not rc.get_retarget_op_enabled(i):
        rc.set_retarget_op_enabled(i, True)
        print("PASS3 re-enabled op", rc.get_op_name(i))
eal.save_loaded_asset(rtg)

# 2) Getting knocked back should read: slower knockback / guard break (were 1.35x and 1.0x -> fell and stood up instantly)
for path, rate in (("/Game/RR/Player/AM_P_Knockback", 0.8), ("/Game/RR/Player/AM_P_GuardBreak", 0.85)):
    m = unreal.load_asset(path)
    m.set_editor_property("rate_scale", rate)
    for attr, t in (("blend_in", 0.12), ("blend_out", 0.3)):
        try:
            b = m.get_editor_property(attr); b.set_editor_property("blend_time", t); m.set_editor_property(attr, b)
        except Exception as e:
            print("PASS3 blend err", e)
    eal.save_loaded_asset(m)
    print("PASS3 rate", path, rate)
print("PASS3_DONE")
