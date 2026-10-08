import unreal, json
out = {}
def cdo(path):
    bp = unreal.load_asset(path)
    return unreal.get_default_object(bp.generated_class())
for gm in ("/Game/AAA/Gameplay/GameModes/BP_AAAGameMode", "/Game/AAA/Gameplay/GameModes/BP_AAAGameMode_Duplicate", "/Game/AAA/Gameplay/BP_AAAGameMode"):
    try:
        c = cdo(gm)
        out[gm] = {k: str(c.get_editor_property(k)) for k in ("default_pawn_class","player_controller_class","hud_class","player_state_class","game_state_class")}
    except Exception as e:
        out[gm] = "ERR " + str(e)[:120]
for pth, props in (("/Game/AAA/Gameplay/GameModes/BP_AAAHUD", ["game_menu_class"]),
                   ("/Game/AAA/Gameplay/GameModes/BP_AAAPlayerController", ["global_ui_mapping_context", "toggle_menu_action"])):
    try:
        c = cdo(pth)
        out[pth] = {p: str(c.get_editor_property(p)) for p in props}
    except Exception as e:
        out[pth] = "ERR " + str(e)[:120]
open(unreal.Paths.project_saved_dir() + "RRTools/inspect4_out.json", "w").write(json.dumps(out, indent=1))
