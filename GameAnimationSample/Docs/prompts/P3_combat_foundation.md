# P3 — Combat foundation: tags, poise, C++ effects, tuning settings, damage pipeline, regen
PROMPT_ID = `P3`. Read `Docs/prompts/00_RULES.md` first. **This session cannot run shell commands: use only your built-in file read / directory listing / text-search tools.** Engine source for API checks: `A:\UNREAL\UE_5.8\Engine\Source` and `...\Engine\Plugins\Runtime\GameplayAbilities`.

## Why this prompt exists (verified facts — trust these, but re-read the files to be sure)
- Damage today: `UAAAGameplayAbility_Melee::OnMontageEventReceived` applies `DamageEffectClass` (an `EditDefaultsOnly TSubclassOf<UGameplayEffect>`, **null by default** → no damage unless a Blueprint subclass sets it) with `SetSetByCallerMagnitude(Data_Damage, -WeaponDamage)` (note the minus sign).
- `UAAAAttributeSet` (`Source/AAAAbility`) has Health, MaxHealth, Stamina, MaxStamina, Armor, Damage (meta), MovementSpeed, Experience, Level. `PostGameplayEffectExecute` already converts the `Damage` meta attribute into a Health reduction. There is **no Poise**, **no stamina/poise regen**, **no block/parry/stagger logic**, and `UAAADamageExecutionCalc` (armor mitigation `Damage*100/(100+Armor)`) is not used by any C++ effect.
- Native tags live in `Source/AAACore/{Public,Private}/AAAGameplayTags.*` accessed as `FAAAGameplayTags::Get().X`. Existing relevant tags: `Data_Damage`, `State_Combat*`, `State_Combat_Invulnerable`, `GameplayEvent_Combat_Hit`, `GameplayCue_Combat_Hit_Melee`, `InputTag_*`.
- `UAAACombatComponent` (`Source/AAACombat`) is the per-pawn combat hub (weapon draw/sheath, stance, `RefreshCombatTimeout`).
- Goal of P3: everything must work **with zero Blueprint/asset setup**. Effects and tuning are defined in C++.

## Deliverables

### 1. Gameplay tags (AAACore)
Add native tags using the **exact existing registration pattern** (member in struct, registration, name string): `State.Combat.Blocking`, `State.Combat.Parrying`, `State.Combat.Staggered`, `State.Combat.Attacking`, `State.Combat.Dead`, `Data.PoiseDamage`, `Data.StaminaCost`, `GameplayEvent.Combat.Blocked`, `GameplayEvent.Combat.Parried`, `GameplayEvent.Combat.Staggered`, `GameplayEvent.Combat.Death`, `GameplayCue.Combat.Hit.Blocked`, `GameplayCue.Combat.Hit.Parried`. Skip any that already exist (grep first). Verify tags are registered **before** any UObject CDO that reads them is constructed (check where registration is invoked, e.g. module `StartupModule`, and the `.uproject` `LoadingPhase`); if there is a risk, make the new effect classes below look tags up lazily (in `PostInitProperties` or on first use) instead of in the constructor, and say so in the report.

### 2. Combat tuning — `UAAACombatSettings : UDeveloperSettings` (AAACombat module)
`UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="AAA Combat"))`, category "Game". Add `DeveloperSettings` to `AAACombat.Build.cs` public deps. `static const UAAACombatSettings* Get()`. All `UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)` with sensible defaults and `ClampMin`:
`BlockDamageReduction=0.75`, `BlockStaminaPerDamage=0.8`, `ParryWindowSeconds=0.25`, `ParryStaggerSeconds=1.6`, `StaggerSeconds=0.9`, `PoiseRegenDelay=3.0`, `PoiseRegenPerSecond=20`, `StaminaRegenDelay=1.0`, `StaminaRegenPerSecond=28`, `LightPoiseDamage=15`, `HeavyPoiseDamage=45`, `HeavyDamageMultiplier=1.8`, `LightStaminaCost=12`, `HeavyStaminaCost=28`, `DodgeStaminaCost=15`, `HitStopLightSeconds=0.045`, `HitStopHeavySeconds=0.11`, `ComboInputBufferSeconds=0.30`, `LockOnRange=1600`, `LockOnMaxAngleDegrees=65`, `PoiseBrokenDamageMultiplier=1.25`.

### 3. Attributes (`UAAAAttributeSet`)
Add `Poise`, `MaxPoise` (replicated, `ReplicatedUsing`, `OnRep_*`, `DOREPLIFETIME_CONDITION_NOTIFY`, default 100/100, clamp 0..MaxPoise in `PreAttributeChange`/`PostGameplayEffectExecute` exactly like Stamina) and meta `IncomingPoiseDamage` (not replicated). **Do not change/remove any existing member.** Update the constructor `Init*` calls.

### 4. C++-defined Gameplay Effects (`Source/AAAAbility/Public/Effects/`, `Private/Effects/`)
- `UAAAGE_Damage : UGameplayEffect` — `Instant`; two modifiers: `Damage` meta attribute `Additive` with magnitude = SetByCaller `Data.Damage` (**positive = damage**), and `IncomingPoiseDamage` `Additive` with SetByCaller `Data.PoiseDamage`. Verify in engine source how `FSetByCallerFloat` / `FGameplayEffectModifierMagnitude(FSetByCallerFloat)` is constructed in 5.8 and how `FGameplayModifierInfo.Attribute` is built from a UProperty (`FindFieldChecked<FProperty>(UAAAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(...))`). Missing SetByCaller should not spam (set `Info` so missing = 0 where the API allows; otherwise always supply both magnitudes at call sites).
- `UAAAGE_StaminaCost : UGameplayEffect` — `Instant`, `Stamina` `Additive`, SetByCaller `Data.StaminaCost` (**caller passes a negative number**; document it).
Keep them tiny. Add `UAAAGE_Damage` as the **default** of `DamageEffectClass` in `UAAAGameplayAbility_Melee`'s constructor (only if it is currently null), and change its call site to send `+DamageAmount` (positive) plus `Data.PoiseDamage` (use `LightPoiseDamage`) — remove the unary minus there. Keep `HitGameplayCueTag` behaviour. (Full melee rewrite happens in the next prompt; here only make it correct and working.)

### 5. Damage pipeline (`UAAAAttributeSet::PostGameplayEffectExecute`, `Damage` branch)
Rewrite that branch (keep the debug log style) to:
1. `Target ASC` + `Source` actor from `Data.EffectSpec.GetContext()`.
2. If target has `State.Combat.Dead` or `State.Combat.Invulnerable` → discard (SetDamage(0), return).
3. If target has `State.Combat.Parrying` → discard damage, send `GameplayEvent.Combat.Parried` to the **source** actor (and `GameplayCue.Combat.Hit.Parried` cue on target), return.
4. Armor mitigation: `Damage * 100/(100+Armor)` (reuse/centralise the formula; do not leave two copies).
5. If target has `State.Combat.Blocking` → `Damage *= (1 - BlockDamageReduction)`, drain `Stamina` by `Damage_before_reduction * BlockStaminaPerDamage`; if stamina hits 0 → clear block (remove loose tag) and force stagger (step 7); send `GameplayEvent.Combat.Blocked` to the target; poise damage ×0.25; cue `GameplayCue.Combat.Hit.Blocked`.
6. If target is **staggered**, multiply damage by `PoiseBrokenDamageMultiplier`.
7. Apply Health reduction (existing code); if Health ≤ 0 and not already dead → add `State.Combat.Dead` (replicated loose tag, authority only) and send `GameplayEvent.Combat.Death` to the target. **Find how `UAAAAttributeComponent` currently detects death (grep `GetGameplayAttributeValueChangeDelegate`, `OnDeath`) and make sure that path still fires exactly once; do not break it.**
8. Poise: consume `IncomingPoiseDamage` (reset it to 0 in a separate branch for that attribute), `Poise -= value`; if Poise ≤ 0 and not already staggered → `UAAACombatComponent::ApplyTimedState(State_Combat_Staggered, StaggerSeconds)`, reset Poise to MaxPoise, send `GameplayEvent.Combat.Staggered` to target.
Server-authoritative only (`Data.Target.AbilityActorInfo->IsNetAuthority()`); presentation reacts to events/cues.

### 6. `UAAACombatComponent` additions (**additive only, keep all existing API**)
- `void ApplyTimedState(FGameplayTag Tag, float Seconds)` — authority only; `AddReplicatedLooseGameplayTag`, timer (weak) to remove it; re-applying refreshes. Also `ClearState(FGameplayTag)`; `bool HasState(FGameplayTag) const`. Clear all timers in `EndPlay`.
- Authority-only regen timer (0.1 s): stamina regen after `StaminaRegenDelay` since last stamina decrease (detect via attribute change delegate on the owning ASC; stop while `State.Combat.Blocking`/`Attacking`), poise regen after `PoiseRegenDelay` since last poise loss. Use `SetNumericAttributeBase`. Do **not** use Tick.
- Initialise safely when the ASC is not yet available (bind on `PossessedBy`/first tick via timer retry; the player's ASC lives on `PlayerState`, an AI pawn's on itself — check `IAbilitySystemInterface` from the owner).

## Acceptance (self-check by reading)
- Zero Blueprint assets needed for: damage, poise, stagger, block/parry arithmetic, regen.
- No existing public function/UPROPERTY removed or re-signed; all new `AAAABILITY_API/AAACOMBAT_API/AAACORE_API` exports right; `Build.cs` deps updated (`DeveloperSettings`, and `AAAAbility`↔`AAACombat` direction must not become circular — if the attribute set needs `UAAACombatComponent`/`UAAACombatSettings`, check the existing dependency graph first; if `AAAAbility` cannot depend on `AAACombat`, move `UAAACombatSettings` to `AAACore` and `ApplyTimedState` calls go through a small interface in `AAACore/Public/Interfaces/` — explain your choice in the report).
- All new tags resolve at runtime; every `SetByCaller` tag used by a GE matches what call sites send.
Then follow the working method and write `Docs/reports/P3_report.md`.
