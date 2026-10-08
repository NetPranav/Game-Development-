# C++ Specs for Antigravity (module `RRGame`)

**Rules for the implementer**
- Read existing code first: `Source/AAACharacter/Public/AAAAPlayerPawn.h`, `AAAAttributeComponent.h`, `Source/AAAAI/Public/AAAATestEnemy.h`, `Source/AAAGame/Public/GameFramework/*.h`, `Source/AAACombat/Public/*`. **Reuse their APIs; do not modify them** unless a spec says so.
- UE **5.8**. Match existing style (tabs, `// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.` header, Doxygen-ish comments only where useful).
- No Blueprint dependency: everything must work from C++ defaults. Do not hardcode asset paths that may not exist; resolve with `ConstructorHelpers`-free soft lookups and fail gracefully (log + continue).
- Every `UFUNCTION`/delegate used from Blueprint must be `BlueprintCallable`/`BlueprintAssignable`.
- Do not run builds. Do not touch `Binaries/`, `Intermediate/`, `.vs/`. Append a line per finished task to `Docs/PROGRESS.md`.
- Server-authoritative where cheap (`HasAuthority()` checks), but single-player PIE must work with no extra setup.

---
## T1 — Module scaffold + Sui bridge  (`Source/RRGame/`)
1. Create module `RRGame` (`RRGame.Build.cs`, `Public/`, `Private/`, `RRGame.h/.cpp` with `IMPLEMENT_MODULE`, `DEFINE_LOG_CATEGORY(LogRR)` declared `RRGAME_API`).
   Deps (Public): `Core, CoreUObject, Engine, InputCore, HTTP, Json, JsonUtilities, UMG, Slate, SlateCore, GameplayTags, GameplayAbilities, AIModule`; (Private): `AAACore, AAACharacter, AAACombat, AAAGame, AAAAI, AAAItems, AAAAbility`.
2. Register: add `RRGame` to `GameAnimationSample.uproject` Modules (`Runtime`, `PreDefault`), and to `ExtraModuleNames` in **both** `Source/GameAnimationSample.Target.cs` and `Source/GameAnimationSampleEditor.Target.cs`.
3. `RRTypes.h`: `UENUM ERRGearKind {Weapon, Armor, Ability}`; `USTRUCT FRRGearData {FString Id; ERRGearKind Kind; FString Name; int64 Seed; int32 Damage, Armor, Power, Level, Wins;}` and `FRRRewardResult {bool bOk; FString Digest, Url, Error; FRRGearData Minted; bool bHasMinted; int32 CreditsAdded;}`.
4. `URRSuiBridge : UGameInstanceSubsystem` implementing the HTTP contract in `SPEC_SUI.md` using `FHttpModule`:
   - `BlueprintCallable`: `CheckHealth()`, `BeginWalletConnect()` (calls `FPlatformProcess::LaunchURL("http://127.0.0.1:3001/connect")` then polls `/session` every 1 s via `FTimerManager` for ≤120 s), `FetchGear()`, `RequestReward(bool bWon, int32 EnemiesKilled)`, `UpgradeWeapon(FString WeaponId)`.
   - State getters: `IsConnected()`, `GetWalletAddress()`, `GetOwnedGear()`, `GetCredits()`.
   - Delegates (dynamic multicast): `OnWalletConnected(FString)`, `OnGearFetched(const TArray<FRRGearData>&)`, `OnRewardResult(const FRRRewardResult&)`, `OnBridgeError(const FString&)`.
   - Base URL is `UPROPERTY(Config)` default `http://127.0.0.1:3001`. Handle non-200 / parse failure → `OnBridgeError`. Use weak `this` captures in lambdas.
   - Console command `rr.sui.test` that runs health → session → gear and logs results.

## T2 — Match flow, enemy, loadout
1. `URRLoadoutComponent : UActorComponent` (on player pawn; add via `GameMode` at runtime if absent): `Equip(const FRRGearData&)`, `GetEquipped(Kind)`, computed `GetDamageMultiplier()` (=1+Weapon.Damage/100 + Ability bonus), `GetDamageReduction()` (=Armor/(Armor+100)), `OnLoadoutChanged`. Default loadout (no wallet) = level-1 starter gear so the game is playable offline.
   Apply to damage: inspect `UAAADamageExecutionCalc` / `UAAAAttributeSet`; the least invasive hook wins (scale the SetByCaller damage at the source in `UAAACombatComponent`/weapon hit code, or set existing attribute base values). Document where you hooked in `PROGRESS.md`.
2. `ARREnemy : AAAAATestEnemy`: Tick-driven kinematic chase toward the nearest player pawn (stop at `AttackRange`), face target, melee attack every `AttackCooldown` that applies damage to the player through the **same damage path the player uses** (`IAAADamageable` / GAS effect — find it), simple dodge sidestep randomly. Exposed `UPROPERTY(EditAnywhere)` tuning values. Snap to ground with a line trace. Broadcast existing death → also notify `ARRGameMode`.
3. `ARRGameMode : AAAAAGameModeBase`: `ERRMatchState {PreMatch, InProgress, Won, Lost}`; on `BeginPlay` find player pawn, ensure `URRLoadoutComponent`, spawn `EnemyCount` (default 3) `ARREnemy` at ring around `ArenaCenter` (default player start) — enemy class `UPROPERTY(EditAnywhere) TSubclassOf<ARREnemy>`; listen to player + enemy death; when all enemies dead → `Won`, call `URRSuiBridge::RequestReward(true, N)` if connected; player dead → `Lost`. `RestartMatch()`. Delegate `OnMatchStateChanged(ERRMatchState)`. Console: `rr.win`, `rr.lose` for testing.

## T3 — HUD + cinematic camera
1. `ARRHUD : AAAAAHUD`: draw with `Canvas` in `DrawHUD()` only (no UMG): player HP/stamina bars, enemy count, equipped gear names + stats, wallet status (`Connected 0x12…ab` / `Press F2 to connect`), owned-gear list (keys `1–9` equips), **Victory/Defeat overlay** with reward text and the Suiscan URL (also logs it and copies to clipboard via `FPlatformApplicationMisc::ClipboardCopy` on Enter). `F2` → `BeginWalletConnect`, `F3` → `FetchGear`, `Enter` on end screen → `RestartMatch`. Bind keys via `PlayerController->InputComponent` raw key bindings (`BindKey`) — no Enhanced Input assets needed.
2. `URRCinematicComponent : UActorComponent` on the player controller: finds `USpringArmComponent` and `UCameraComponent` on the pawn; **dueling camera** (frame player + nearest enemy: arm length `Clamp(Dist*0.9+350, 450, 900)` with `FInterpTo`), **hit-stop** `HitStop(Duration, Dilation=0.1)` via `SetGlobalTimeDilation` + unscaled timer, **dash FOV** widen, **damage pulse** (post-process vignette/chromatic aberration via camera `PostProcessSettings`), **finisher** on last-enemy kill: dilation 0.2, camera orbits the corpse 1.5 s with a timer-driven yaw, then `GameMode` announces Won. Hook into `OnHealthChanged`/death delegates; expose `BlueprintCallable` triggers.

## T4 — Robot visuals (after T1–T3 verified)
`URRRobotAppearanceComponent`: from `Seed` derive hue/emissive/metallic; create `UMaterialInstanceDynamic` on every mesh material slot of the owner's skeletal mesh; scalar/vector params `BaseHue`, `EmissiveGlow`, `Metallic` (parameter names configurable; no-op if the material lacks them). Apply from equipped gear on `OnLoadoutChanged`; enemy uses a random seed.
