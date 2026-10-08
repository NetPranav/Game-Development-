import unreal, json, traceback
log = {}
eal = unreal.EditorAssetLibrary
def tune(path, blend_in, blend_out, rate):
    m = unreal.load_asset(path)
    bi = m.get_editor_property("blend_in"); bi.set_editor_property("blend_time", blend_in); m.set_editor_property("blend_in", bi)
    bo = m.get_editor_property("blend_out"); bo.set_editor_property("blend_time", blend_out); m.set_editor_property("blend_out", bo)
    m.set_editor_property("rate_scale", rate)
    try: m.set_editor_property("blend_out_trigger_time", -1.0)
    except Exception: pass
    eal.save_loaded_asset(m)
    return "%s in=%.2f out=%.2f rate=%.2f len=%.2f" % (m.get_name(), blend_in, blend_out, rate, m.get_editor_property("sequence_length"))
for n, bi, bo, r in (("AM_P_Light1", 0.06, 0.18, 1.15), ("AM_P_Light2", 0.06, 0.18, 1.15), ("AM_P_Light3", 0.08, 0.25, 1.05),
                     ("AM_P_Heavy", 0.10, 0.25, 1.0), ("AM_P_Block", 0.08, 0.15, 1.0),
                     ("AM_P_HitReact_Front", 0.04, 0.15, 1.2), ("AM_P_HitReact_Back", 0.04, 0.15, 1.2),
                     ("AM_P_HitReact_Left", 0.04, 0.15, 1.2), ("AM_P_HitReact_Right", 0.04, 0.15, 1.2),
                     ("AM_P_Stun", 0.05, 0.2, 1.0), ("AM_P_Death", 0.1, 0.0, 1.0)):
    try: log[n] = tune("/Game/RR/Player/" + n, bi, bo, r)
    except Exception as e: log[n] = "ERR " + str(e)[:100]
open(unreal.Paths.project_saved_dir() + "RRTools/tune_out.json", "w").write(json.dumps(log, indent=1))
