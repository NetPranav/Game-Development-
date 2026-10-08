# Robot Rumble — how to run, play, test, rebuild

## Run
1. Open `GameAnimationSample.uproject` (UE 5.8). The editor opens **`/Game/RR/Levels/L_RRArena`** (set as EditorStartupMap and GameDefaultMap).
2. Press **Play**. A 3-second countdown, then 3 Greystone enemies spawn around you. Win by killing them all; lose by dying. `Enter` restarts.
3. If you ever see "Derives from invalid class" after pulling C++ changes: close the editor, rebuild (see below), reopen.

## Controls (no assets/Enhanced Input needed for combat)
| Key | Action |
|---|---|
| WASD / mouse | move / camera (Game Animation Sample locomotion, untouched) |
| **LMB** | light attack: click 1 = fast slash, click 2 = second slash, click 3 = finisher (more damage, knockback, stagger). Click again within ~1.2 s to continue; the swing only starts on a click. Input is buffered. |
| **F** | heavy attack |
| **RMB** (hold) | block; **tap just before a hit = parry** (attacker is staggered) |
| **Left Shift** | dodge (i-frames in the middle of the roll) |
| **Q** / MMB | lock on / off, `[` `]` switch target |
| **R** | draw / sheath weapon |
| **Tab** | inventory (pauses): click an item to equip, right-click to unequip, mouse wheel scrolls, Esc/Tab closes; GEAR / WALLET tabs |
| **F2** | connect Sui wallet (opens http://127.0.0.1:3001/connect) |
| **F3** | fetch owned gear; **1–9** equip gear slot n |
| **Enter** | restart match (end screen) · **C** copy the reward tx link |

## Console commands (`~`)
`rr.enemy.spawn [n] [dist]`, `rr.enemy.killall`, `rr.test.autoplay` (bot that fights), `rr.test.shot`, `rr.ai.debug 1|2`,
`rr.win`, `rr.lose`, `rr.match.restart`, `rr.sui.test`, `rr.combat.hurt <dmg> [poise]`, `rr.combat.stagger`, `rr.combat.setstamina <v>`.
Tuning: **Project Settings → Game → AAA Combat** (damage multipliers, stagger/parry timing, hit-stop, knockback, lock-on range…) and **RR Game Settings**.

## What is wired where
- C++: module `RRGame` (match, enemy AI, HUD, loadout, Sui bridge) + extended `AAACombat/AAAAbility/AAAGame` (combat core). See `Docs/PLAN.md`, `Docs/SPEC_CPP.md`.
- Content created by scripts: `/Game/RR/Greystone/*` (enemy weapon data, montages, IK rig, retargeter), `/Game/RR/Player/*` (retargeted combat anims, player montages, `DA_PlayerWeapon`), `/Game/RR/Blueprints/{BP_RREnemy_Greystone,BP_RRGameMode,BP_RRHUD}`, `/Game/RR/Levels/L_RRArena`.
- Scripts that built them: `Tools/editor_scripts/*.py` — re-runnable headless:
  `UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<file.py> -unattended -nosplash -nullrhi`

## Rebuild (editor closed)
```
"A:\UNREAL\UE_5.8\Engine\Build\BatchFiles\Build.bat" GameAnimationSampleEditor Win64 Development -Project="A:\UNREAL PROJECT\Game\GameAnimationSample\GameAnimationSample.uproject" -WaitMutex -NoHotReload
```
Headless gameplay smoke test (no window): add `-game -nullrhi -nosound -unattended -log -forcelogflush -benchmark -benchmarkseconds=60 -fps=30 -ExecCmds="rr.test.autoplay"` and read the log for `[DAMAGE]`, `LogRR`.

## Sui (blockchain) side
1. Contract: `Sui/contracts` (`sui move build` passes). Publish to testnet: `sui client publish --gas-budget 200000000` from `Sui/contracts`; note PackageID, `AdminCap` object id, shared `Vault` id.
2. Sidecar: `cd Sui/sidecar && copy .env.example .env` → fill `PACKAGE_ID, ADMIN_CAP_ID, VAULT_ID, ADMIN_PRIVATE_KEY` (**never commit `.env`**), then `npm start`.
3. In game: F2 → connect wallet in the browser page → win a match → reward is minted on testnet and the Suiscan link appears on the victory screen.

## Known gaps / next
- Sui contract not yet published (needs testnet SUI from https://faucet.sui.io for the active address) → reward flow untested end-to-end.
- Sword holster pose (sheathed on the back) is untuned; the hand grip/rotation is tuned (`DA_PlayerWeapon` HandOffset, `Grip` socket on the sword meshes).
- Robot visuals from gear seed (`URRRobotAppearanceComponent`) not implemented yet.
- Retargeted player animations need a visual quality pass in the editor (slot node for hit-react/block montages depends on the Game Animation Sample AnimBP).

## Player visuals (robot) and 4 GB-VRAM note
- The player body is the Fab **Robot scout R21** (`/Game/RR/Blueprints/BP_RobotScout`, a clone of the sample's `BP_UE4_Mannequin` display actor). `URRRobotVisualComponent` swaps it in at runtime and hides the master mannequin (which still drives the pose), so every locomotion/combat animation is retargeted live; nothing in the AnimBP changed.
- Robot and sword textures were capped to 1024 px (`Tools/editor_scripts`); `r.Streaming.PoolSize=1200`. Without this a laptop GPU ran out of video memory.
- Combat-feel tuning lives in **Project Settings → Game → AAA Combat → Feel** (combo damage per click, lunge, aim assist, window fractions).

## Front end, defeat, inventory (added after the combat pass)
- Flow: splash (logo, KAIZEEN, quote; click/Enter skips) -> start menu (CONTINUE / NEW GAME / BLOCKCHAIN / SETTINGS / QUIT) -> loading screen (art slideshow, black bar, tips) -> countdown + match. `-RRSkipMenu` on the command line skips all of it (used by the test scripts).
- Art: `Images/` is imported to `/Game/RR/UI` by `Tools/editor_scripts/import_ui_images.py` (Starting.png = menu, the 4 jpgs = loading). Optional team logo: put `Kaizeen_logo.png` in `Images/`, re-run the script (asset `T_KaizeenLogo`); otherwise a procedural emblem is drawn.
- Start point: move the level's **PlayerStart** actor in the editor; the player spawns and respawns there. `rr.start.here` sets a runtime respawn point at the player's position.
- Defeat: screen fades to black, white DEFEAT, stats (damage dealt/taken, kills, time, weapon, scrap earned), then auto-respawn (Enter skips). Test: `rr.ui.defeat [delaySeconds]`.
- Inventory (Tab): Resident-Evil-2 style, equipped slots + condition monitor + 4-wide pouch grid; WASD/arrows, Enter equip/unequip, Backspace/RMB unequip, Q/E tabs, mouse hover/click/wheel. Test: `rr.ui.inventory [delay]`, `rr.ui.screen splash|menu|settings|chain|loading|playing`.
- Build tip: if UBT fails with "user-mapped section" on `*.uhttimestamps`/`DependencyCache.bin`/`SourceFileCache.bin`, delete those three files and rebuild.
