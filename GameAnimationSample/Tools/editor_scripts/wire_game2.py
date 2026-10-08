import unreal, json, traceback
log = {}
at = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary

def step(name):
    def deco(fn):
        try:
            r = fn(); log[name] = "ok" if r is None else r
        except Exception as e:
            log[name] = "ERR " + str(e)[:300] + " | " + traceback.format_exc()[-300:]
        return fn
    return deco

def compile_save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)

@step("hud_bp")
def _():
    path = "/Game/RR/Blueprints/BP_RRHUD"
    if eal.does_asset_exist(path):
        bp = unreal.load_asset(path)
    else:
        f = unreal.BlueprintFactory(); f.set_editor_property("parent_class", unreal.RRHUD)
        bp = at.create_asset("BP_RRHUD", "/Game/RR/Blueprints", unreal.Blueprint, f)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    menu = unreal.load_asset("/Game/AAA/UI/Widgets/WBP_GameMenu")
    cdo.set_editor_property("game_menu_class", menu.generated_class())
    compile_save(bp)

@step("gamemode_classes")
def _():
    bp = unreal.load_asset("/Game/RR/Blueprints/BP_RRGameMode")
    cdo = unreal.get_default_object(bp.generated_class())
    G = "/Game/AAA/Gameplay/GameModes/"
    cdo.set_editor_property("player_controller_class", unreal.load_asset(G + "BP_AAAPlayerController").generated_class())
    cdo.set_editor_property("player_state_class", unreal.load_asset(G + "BP_AAAPlayerState").generated_class())
    cdo.set_editor_property("game_state_class", unreal.load_asset(G + "BP_AAAGameState").generated_class())
    cdo.set_editor_property("hud_class", unreal.load_asset("/Game/RR/Blueprints/BP_RRHUD").generated_class())
    compile_save(bp)

@step("tune_enemy_weapon")
def _():
    da = unreal.load_asset("/Game/RR/Greystone/DA_GreystoneWeapon")
    combat = da.get_editor_property("combat")
    combat.set_editor_property("base_damage", 10.0)
    da.set_editor_property("combat", combat)
    eal.save_asset("/Game/RR/Greystone/DA_GreystoneWeapon")

open(unreal.Paths.project_saved_dir() + "RRTools/wire2_out.json", "w").write(json.dumps(log, indent=1, default=str))
