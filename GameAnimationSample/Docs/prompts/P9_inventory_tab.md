# P9 — Inventory screen inside the Tab menu (canvas-drawn, mouse-driven) + gear equip with weapon visuals
Read first: `Docs/prompts/00_RULES.md` (including "Lessons from the first real compile"), `Docs/reports/MODULE_DEPS.md`, `Docs/reports/FILE_INDEX.md`. **Never call run_command or any shell tool — a denied shell call ends your session. No directory listing is needed; all paths are named. Do not delegate to subagents.** Verify engine APIs by viewing engine source by path under `A:/UNREAL/UE_5.8/Engine`.

## Goal (what the user asked for)
"I want the inventory in the Tab window": when the player presses **Tab**, a full inventory screen opens (game paused, mouse cursor visible) where gear can be browsed and equipped **with the mouse**. Everything is drawn with `AHUD`/`Canvas` calls (no UMG, no Slate brushes: see `Docs/after/BUGS.md` BUG-02). The existing placeholder widget (`WBP_GameMenu`, 5 empty grey tabs) must no longer be spawned.

## Verified facts (re-read the files to confirm)
- Tab → `AAAAAPlayerController::OnToggleMenu()` (`Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`): calls `AAAAAHUD::ToggleGameMenu()` (`Source/AAAGame/Public/GameFramework/AAAAHUD.h`, spawns `GameMenuClass`), and if it returns true sets `FInputModeGameAndUI`, `SetShowMouseCursor(true)`, `SetPause(true)`; if false it restores game input and unpauses. `AAAAAHUD::DrawHUD` draws damage numbers (P6b). `AAAGame` must not depend on `RRGame`/`AAACharacter`.
- `ARRHUD : AAAAAHUD` (`Source/RRGame/{Public,Private}/RRHUD.*`) is canvas-only: vitals bars, enemy bars, match banners, a top-right wallet/gear panel, keys F2 (wallet), F3 (fetch gear), 1–9 (equip owned gear n), Enter, C. Read it fully first; keep all existing behaviour.
- `URRLoadoutComponent` (`RRLoadoutComponent.*`): holds equipped `FRRGearData` per `ERRGearKind`, `Equip`, `Unequip`, `OnLoadoutChanged`, applies stats via `UAAACombatComponent::SetLoadoutModifiers`. `FRRGearData` (`RRTypes.h`): Id, Kind (Weapon/Armor/Ability), Name, Seed (int64), Damage, Armor, Power, Level, Wins.
- `URRSuiBridge` (`RRSuiBridge.*`): `GetOwnedGear()`, `GetCredits()`, `IsConnected()`, `GetWalletAddress()`, `BeginWalletConnect()`, `FetchGear()`, delegates `OnGearFetched`, `OnWalletConnected`.
- `ARRGameMode` has `UPROPERTY TObjectPtr<UAAAWeaponSettings> PlayerWeaponSettings` (weapon data asset with montages and `Visuals.StaticMesh`), used with `UAAACombatComponent::EquipWeapon(UAAAWeaponSettings*)`. A weapon actor is spawned from the settings' `Visuals.StaticMesh`; the swords' pivot is handled by a `Grip` socket on the mesh (already done).
- Sword meshes: `/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A` and `/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B`.

## Deliverables

### 1. `AAAAAHUD::ToggleGameMenu` becomes `virtual` (only change in AAAGame: add `virtual`; keep behaviour). `ARRHUD` overrides it: toggles `bInventoryOpen` and **does not** spawn any widget; returns the new open state so the controller keeps pausing/showing the cursor. Also close on **Escape** (bind with `bExecuteWhenPaused = true`, only while open) by calling the same toggle logic through the controller (`OnToggleMenu` is `protected` — add a small public `CloseMenuIfOpen()` helper on `AAAAAPlayerController` or call `ToggleGameMenu` then replicate the controller's un-pause/input-mode restore in one place; do not duplicate large logic).

### 2. Local inventory data (`RRLoadoutComponent` or a new `RRInventory.h/.cpp` in `RRGame`)
`FRRInventoryItem { FRRGearData Gear; bool bOnChain; int32 VisualVariant; }`. `TArray<FRRInventoryItem> BuildInventory()` = local items + on-chain items from `URRSuiBridge::GetOwnedGear()` (`bOnChain = true`). Local items (fixed ids `local_*`): weapons "Rustic Blade" (dmg 12, starter), "Dark Sword I" (dmg 18, variant 0 = Sword A), "Dark Sword II" (dmg 24, variant 1 = Sword B); armors "Padded Frame" (armor 8), "Plated Chassis" (armor 20); abilities "Overdrive" (power 15). `VisualVariant` for on-chain weapons = `Seed % 2`.

### 3. Equipping a weapon changes the sword the robot holds
In `URRLoadoutComponent` (authority/local single-player is enough): when a **Weapon** is equipped, build a runtime copy of the player's weapon data (`DuplicateObject` of `ARRGameMode::PlayerWeaponSettings`, kept alive in a `UPROPERTY` field), set `Visuals.StaticMesh` to Sword A or B by `VisualVariant` ("Rustic Blade" keeps Sword A), `Combat.BaseDamage` = `12 + Damage*0.5` (document formula; keep multipliers from the existing loadout math consistent — do not double count with `SetLoadoutModifiers`: decide one place, state it in the report), and call `UAAACombatComponent::EquipWeapon` with it, re-drawing the weapon if it was drawn. Soft-load the meshes through a `TSoftObjectPtr<UStaticMesh>` pair on `URRSettings` (`RRSettings.h`, add `SwordMeshes`), defaults set in `RRSettings.cpp` to the two paths above.

### 4. The inventory screen (`ARRHUD::DrawInventory`, called from `DrawHUD` when open; skip the normal in-match panels that would overlap while open)
Layout in canvas fractions (works at any resolution, min 960×540):
- Dim the whole screen (semi-transparent black), a central panel (≈ 80 % × 78 %) with title **INVENTORY**, tabs **GEAR | WALLET** (clickable via hit boxes; WALLET tab shows connection state, short address, credits, buttons *Connect (F2)* and *Refresh (F3)* that call the bridge).
- GEAR tab: left column **EQUIPPED** with three slots (Weapon, Armor, Ability) each showing name, stat line and a coloured swatch derived from the seed (`FLinearColor::MakeFromHSV8` from seed bits); right area a scrollable grid of item cards (5 columns, card ≈ 15 % width): kind icon drawn as a simple shape/letter, name, main stat, `Lv`, an **ON-CHAIN** badge for NFTs, equipped items outlined in gold. Mouse wheel scrolls (bind `MouseScrollUp/Down` on the controller `InputComponent` with `bExecuteWhenPaused = true`, only while open); a scrollbar line on the right.
- Hovering a card shows a details strip at the bottom (name, kind, all stats, level, wins, seed, shortened object id, `Left click: equip   Right click: unequip   Esc: close`).
- Left click equips, right click unequips that kind; an equipped-slot click unequips. Number keys 1–9 still equip the n-th item of the current inventory order.
- Interaction: use `AHUD::AddHitBox(Position, Size, Name, bConsumesInput=true, Priority)` every frame for tabs, buttons and cards and handle `NotifyHitBoxClick`/`NotifyHitBoxBeginCursorOver`/`EndCursorOver` (set `PC->bEnableClickEvents = true` and `bEnableMouseOverEvents = true` while open). **If you determine from the engine source that hit-box clicks are not delivered while the game is paused, implement click handling by polling the cursor** (`PC->GetMousePosition`) and `PC->WasInputKeyJustPressed(EKeys::LeftMouseButton/RightMouseButton)` inside `DrawHUD` instead (explain which path you chose and why in the report).
- Fonts: `GEngine->GetLargeFont()/GetMediumFont()/GetSmallFont()`, `DrawText(...)` with scale; shapes with `DrawRect`/`DrawLine`. **No Slate brushes, no NewObject per frame.** Never crash if the pawn, loadout component, bridge or game mode is missing (draw a "no data" message).

### 5. Housekeeping
- Add `bInventoryOpen` handling so that `Enter`, `C`, `F2`, `F3` and digit keys do not fire gameplay actions twice; the in-match panels (wallet/gear top-right) are hidden while the inventory is open.
- Keep `Source/RRGame/RRGame.Build.cs` correct (no new module cycles; `RRGame` already depends on `AAAGame`).

## Acceptance (self-check by reading)
- Tab opens/closes the screen (pause + cursor via the existing controller code), Escape closes it, nothing else spawns a widget.
- Every item in the inventory can be equipped with the mouse; equipping a weapon swaps the sword mesh (A/B); equipping armor/ability updates the existing loadout stats; all HUD code is null-safe.
- Report `Docs/reports/P9_report.md` per `00_RULES.md` with honest self-review ticks, the three riskiest spots, and exact asset/property names the reviewer must set (e.g. `BP_RRHUD.GameMenuClass` should be cleared).
Then append `P9 done` to `Docs/PROGRESS.md`.
