# P5 — Block, parry, Mover-compatible dodge + shared "move a Mover pawn" utility
PROMPT_ID = `P5`. Read `Docs/prompts/00_RULES.md`, `Docs/reports/P3_report.md`, `Docs/reports/P4_report.md` (P3+P4 are landed). **No shell commands — built-in read/list/search/edit tools only.** Verify every engine API in `A:\UNREAL\UE_5.8\Engine` (the Mover plugin source is under `...\Engine\Plugins\Experimental\Mover` or `...\Plugins\Runtime\Mover` — find it).

## Verified facts (re-read to confirm)
- The player pawn `AAAAAPlayerPawn` (`Source/AAACharacter`) derives from **`APawn`** (not `ACharacter`) and moves with Epic's **Mover** plugin (`UMoverComponent` is added by the Blueprint `BP_PlayerCharacter`, NOT by the C++ class → find it at runtime with `FindComponentByClass<UMoverComponent>()`). There is **no `UCharacterMovementComponent`** on it.
- `UAAAGameplayAbility_Dodge` (`Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Dodge.cpp`) does `Cast<ACharacter>(Avatar)` and **ends immediately if the cast fails** → dodge can never run on the player. It also moves via `UCharacterMovementComponent::AddImpulse` (absent on Mover pawns) and defines `StaminaCost` that is never applied. It grants `State_Combat_Invulnerable` as a loose tag for the whole ability.
- Controller input: `AAAAAPlayerController::SetupInputComponent` binds R / LMB / F (heavy, added in P4) / LShift via `InputComponent->BindKey`.
- P3 pipeline already handles `State.Combat.Blocking` (damage reduction, stamina drain, block-break stagger) and `State.Combat.Parrying` (discard damage + `GameplayEvent.Combat.Parried` sent to the **attacker's** ASC via `HandleGameplayEvent`). Nothing *sets* those tags yet; nothing listens to `Parried`.

## Deliverables

### 1. `UAAAMoverUtils` (BlueprintFunctionLibrary, module `AAACharacter` or wherever it avoids cycles; export macro correct)
- `static bool ApplyVelocityBurst(AActor* Pawn, FVector WorldVelocity, float DurationSeconds)`: if the actor has a `UMoverComponent`, queue the correct **instant movement effect** from the Mover plugin that sets/launches velocity (study how the plugin does it: `QueueInstantMovementEffect`, `FApplyVelocityEffect` / `FLaunchEffect`-style structs — use whatever 5.8 actually provides, with the plugin's own sample code as reference); else if `ACharacter`, use `LaunchCharacter`; else fall back to a short **kinematic slide** (timer-driven `SetActorLocation(Sweep)` for `DurationSeconds`, weak capture) so it still works. Return false only if nothing was possible.
- `static FVector GetPlanarVelocity(AActor*)` (Mover velocity → `GetVelocity()` fallback) and `static FVector GetMoveIntent(AActor*)` (last input intent if Mover exposes it, else normalized planar velocity, else zero). Document what you found.
- Add the `Mover` module to the needed `Build.cs` dependencies (module name: verify in the plugin's `.Build.cs`; also check the plugin is enabled in `GameAnimationSample.uproject` — it is).

### 2. Block & parry on `UAAACombatComponent` (additive)
- `UFUNCTION(BlueprintCallable) void SetBlocking(bool bWantsBlock)` → if not authority call `Server_SetBlocking(bool)` (Reliable, `WithValidation` optional). Server rules for `true`: allowed only if not Dead/Staggered/Attacking and `Stamina > 0`; auto-draw weapon if sheathed; add replicated loose `State.Combat.Blocking`; **also** add `State.Combat.Parrying` for `ParryWindowSeconds` (from `UAAACombatSettings`) via `ApplyTimedState`, but only if the block key was pressed fresh (not re-entering within 0.4 s of a previous parry window, to prevent hold-spam: keep `LastParryWindowStart`). For `false`: remove `Blocking` and `Parrying` immediately.
- Block drains nothing while idle; it ends automatically if stamina reaches 0 (the P3 attribute-set code clears the tag — make sure the component's `bWantsBlock` bookkeeping is also reset by listening to tag removal on the owning ASC).
- Bind on the owner's ASC: `GenericGameplayEventCallbacks` for `GameplayEvent.Combat.Parried` → the **attacker** (this component's owner) gets `ApplyTimedState(State_Combat_Staggered, ParryStaggerSeconds)`; cancel any active melee ability (cancel by `State.Combat.Attacking`/ability tag). Bind in the same place regen is initialised (retry-safe when the ASC is not available yet). Unbind in `EndPlay`.
- Optional animation hook: extend `UAAAWeaponSettings::Animations` with `TObjectPtr<UAnimMontage> BlockMontage` and `ParryMontage` (**optional**; code must work when null). When blocking starts, play `BlockMontage` looping via the avatar's AnimInstance (`Montage_Play` + jump to a loop section if one named `Loop` exists), stop on release. When both absent do nothing visible (the HUD will show state).
- Input: in `AAAAPlayerController` bind **Right Mouse Button** pressed → `SetBlocking(true)`, released → `SetBlocking(false)` via `BindKey` (`IE_Pressed` / `IE_Released`). Auto-release block when the pawn is unpossessed / dies.

### 3. Rewrite `UAAAGameplayAbility_Dodge` (keep class name, UPROPERTY names, `InputTag.Dodge` activation, `LocalPredicted`)
- Works for **any `APawn`** (Mover) — no `ACharacter` requirement.
- Costs: apply `UAAAGE_StaminaCost` (negative SetByCaller `Data.StaminaCost` = `-StaminaCost`, default from `UAAACombatSettings::DodgeStaminaCost` when the ability property is 0); refuse to activate if stamina < cost (`CanActivateAbility`) and when Staggered/Dead.
- Direction: `UAAAMoverUtils::GetMoveIntent` (relative to actor facing: 4-way pick as today; no intent → backstep). Movement: `ApplyVelocityBurst(Pawn, Dir * (DodgeImpulseStrength), DodgeDuration)` — treat `DodgeImpulseStrength` as cm/s speed and **tune sensible defaults** (e.g. 900 cm/s for 0.3 s → ~2.7 m). Montages (4 directions) optional exactly as today.
- **I-frames only for the middle of the dodge**: `Invulnerable` from 0.05 s to 0.30 s (new `IFrameStart/IFrameEnd` UPROPERTYs), implemented with timers that add/remove the loose tag (authority and local), always removed in `EndAbility`. Total duration `DodgeDuration` default 0.45 s with a recovery tail during which another dodge is blocked.
- Dodge **cancels** an active melee attack (use ability cancel tags / `CancelAbilitiesWithTag`) and clears the combo buffer; it can't interrupt Staggered.
- Remove the `RequestGameplayTag("InputTag.Dodge", false)` constructor lookup if tags are guaranteed initialised (they are: `FAAACoreModule::StartupModule` initialises native tags); use `FAAAGameplayTags::Get().InputTag_Dodge`.
- `EndAbility` must be safe to call twice and from every path.

### 4. Tiny debug aid
Console commands registered from `UAAACombatComponent` (or a subsystem) for testing without animations: `rr.combat.hurt <damage> [poise]` (applies `UAAAGE_Damage` to the local player pawn's ASC from itself), `rr.combat.stagger`, `rr.combat.setstamina <v>`. Use `FAutoConsoleCommandWithWorldAndArgs`; guard `#if !UE_BUILD_SHIPPING`.

## Acceptance (self-check by reading)
- Dodge works on a pawn with no `ACharacter`; i-frames window correct; tag always cleaned.
- RMB press→block+parry window, release→clear; blocked hits cost stamina; parried hit staggers the attacker; tags are replicated loose tags added on authority only.
- No existing public API removed; Build.cs deps correct; no cycles; every timer weak-captured and cleared. Report per `00_RULES.md` with the three riskiest spots.
