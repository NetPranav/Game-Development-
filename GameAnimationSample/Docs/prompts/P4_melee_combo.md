# P4 — Melee: input buffer, combo chain, heavy attack, blade sweep, team filter, montage-less fallback
PROMPT_ID = `P4`. Read `Docs/prompts/00_RULES.md`, then `Docs/reports/P3_report.md` (P3 already landed: C++ effects `UAAAGE_Damage`/`UAAAGE_StaminaCost`, `UAAACombatSettings`, poise, new tags, `UAAACombatComponent::ApplyTimedState/HasState`). **No shell commands: use built-in read/list/search/edit tools only.** Verify engine APIs in `A:\UNREAL\UE_5.8\Engine`.

## Verified facts about the current code (re-read to confirm)
- `AAAAAPlayerController::OnAttackAction` (`Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`) calls `ASC->TryActivateAbilitiesByTag(InputTag_Attack)`. Keys are bound with `InputComponent->BindKey` (R draw/sheath, LMB attack, LShift dodge).
- `UAAAGameplayAbility_Melee` (`Source/AAAAbility`) is `InstancedPerActor`, picks `WeaponSettings->Animations.LightAttackCombos[CurrentComboIndex]` (hard `UAnimMontage*` array), plays it with `UAbilityTask_PlayMontageAndWait`, waits for `GameplayEvent_Combat_Hit`, applies damage on authority. Pressing LMB while the ability is active does **nothing** (no buffering, no early chain). There is no heavy attack.
- `AAAAWeaponActor::PerformTrace` (`Source/AAACombat/Private/AAAAWeaponActor.cpp`) ticks while `bIsTracing`; it sweeps only the **blade tip** (`TraceEndSocket`) from last frame to this frame with a 20 cm sphere, computes `StartLocation` but never uses it, has no team/dead filtering, and sends `GameplayEvent.Combat.Hit` to the owner.
- `AnimNotifyState_WeaponTrace` (`Source/AAAAnimation`) calls `UAAACombatComponent::StartWeaponTrace/Stop...`. **No attack montages exist in the project yet** (they will be imported later: Paragon Greystone, Mixamo, etc.), and imported montages will not have our notifies authored. Therefore the code must work **with or without** notifies and **with or without** montages.
- `UAAAWeaponSettings` (`Source/AAACombat/Public/DataAssets/AAAWeaponSettings.h`): `Combat.{BaseDamage, AttackSpeedMultiplier, Range, StaminaCost, Weight}`, `Animations.{LightAttackCombos, HeavyAttackCombos?, Equip, Unequip}` — read the real struct and use what exists.

## Design

### Input → `UAAACombatComponent` (single entry point, replaces direct ability activation from the controller)
- `enum class EAAAAttackType : uint8 { Light, Heavy }`.
- `void RequestAttack(EAAAAttackType Type)` (BlueprintCallable, called locally by the controller):
  - If not currently attacking → try to activate the melee ability (`InputTag_Attack` for Light, new tag `InputTag.AttackHeavy` for Heavy — add the tag like other native tags; add a thin `UAAAGameplayAbility_MeleeHeavy` **only if** the same class cannot serve both; prefer ONE ability class that reads the requested type from the component).
  - If attacking → store `BufferedAttack = Type` and `BufferExpiresAt = World time + ComboInputBufferSeconds` (from `UAAACombatSettings`). Latest press wins.
  - `bool ConsumeBufferedAttack(EAAAAttackType& OutType)`; `bool IsAttacking() const` (from the `State.Combat.Attacking` tag).
- Update `AAAAPlayerController`: LMB → `RequestAttack(Light)`; add `F` key → `RequestAttack(Heavy)`. Keep the existing LMB/R/Shift bindings otherwise. Leave `OnAttackAction` as a thin wrapper.

### `UAAAGameplayAbility_Melee` rewrite (keep class name, keep the existing UPROPERTYs, add what is needed)
Behaviour:
1. `CanActivateAbility`/commit: require stamina ≥ cost (`Light/HeavyStaminaCost` from settings, scaled by weapon `Combat.StaminaCost` if > 0 — document formula); on activation apply `UAAAGE_StaminaCost` to self with a **negative** SetByCaller `Data.StaminaCost`. Block activation if owner has `State.Combat.Staggered`, `.Dead`, `.Blocking` (use the 5.8-correct blocked-tags API; verify in `GameplayAbility.h`).
2. Grant `State.Combat.Attacking` while active (use the non-deprecated 5.8 mechanism; verify), auto-draw the weapon if sheathed (keep existing behaviour).
3. **Combo chain in one ability instance**: attack index `i` (light chain from `LightAttackCombos`, wraps), heavy uses `HeavyAttackCombos[0]` (if the array does not exist/empty, reuse the last light montage with 1.4× play-rate and heavy damage). Combo index resets to 0 if the chain is idle longer than `ComboResetSeconds` (add to settings, default 1.2) or the ability is cancelled by stagger.
4. **Combo window** — when the current attack reaches its window, if `ConsumeBufferedAttack` returns true: stop the current montage with a short blend (0.12 s), end hit tracing, and start the next attack **in the same ability instance** (no re-activation, so stamina is charged per attack). The window is defined by the new notify state `UAAAAnimNotifyState_ComboWindow` (`Source/AAAAnimation`, same pattern as `AnimNotifyState_WeaponTrace`, talking to the combat component) **or, if the montage contains no such notify state, by a fallback**: open at 45 % and close at 95 % of the montage play length (timers, weak captures, scaled by play rate). Same fallback rule for tracing: if the montage has no `AnimNotifyState_WeaponTrace`, the ability itself starts tracing at 20 % and stops at 55 % of play length. Detect notify presence by scanning `UAnimMontage::Notifies` (and the composite sections' sequences are out of scope).
5. **No-montage fallback** (weapon settings have none, e.g. fresh project): run a *procedural attack* of `ProceduralAttackSeconds` (settings, default 0.55): same stamina/state/window logic, trace window 20–55 %, hits found with a forward capsule/box sweep in front of the avatar (range = weapon `Combat.Range` or 180 cm) instead of the weapon actor. Log once: "no montage for attack X, using procedural fallback". This makes combat testable before any animation exists.
6. Damage = `BaseDamage × (Heavy ? HeavyDamageMultiplier : 1) × (1 + 0.1 × comboStep) × CombatComponent->GetDamageMultiplier()`; poise damage = Light/HeavyPoiseDamage. Applied via `UAAAGE_Damage` with **positive** SetByCaller values on **authority only** (the existing `HasAuthority` check). Non-authority paths only play cosmetics.
7. Always `EndAbility` exactly once; end on montage end/interrupt, stagger, death; clear timers & remove the `Attacking` state and stop tracing on every exit path (`EndAbility` override). Cancel when `State.Combat.Staggered` or `.Dead` is added (bind to `ASC->RegisterGameplayTagEvent` or use cancel tags — verify).

### `UAAACombatComponent` additions
- `float DamageMultiplier = 1.f; float DamageReduction = 0.f;` (replicated, BlueprintReadWrite, with `SetLoadoutModifiers(float DamageMult, float DamageReduct)` on authority). `GetDamageMultiplier()` / `GetDamageReduction()`. (The Sui loadout later sets these.) Apply `DamageReduction` in the attribute-set damage pipeline from P3 (one extra multiply: `Damage *= (1 - DamageReduction)`) — find the target's combat component via the avatar actor.
- Delegate `FAAAOnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)` (dynamic multicast, BlueprintAssignable) broadcast on **every machine** that sees a confirmed hit (authority broadcasts after applying damage; for single-player this is the same machine). Presentation (hit-stop, shake) will bind to it in a later prompt.

### `AAAAWeaponActor` fixes
- Sweep the whole blade: sample `NumBladeSamples=3` points between `TraceStartSocket` and `TraceEndSocket` (start, mid, end), sweeping each from its previous-frame position to the current one (keep an array of previous positions; reset on `StartWeaponTrace`). Use the existing radius (20 cm) as a `UPROPERTY(EditAnywhere)` `TraceRadius`. Remove the unused variable.
- Filter targets: skip the owner & its attached actors, actors with `State.Combat.Dead`, and **same-team** actors — find the real team API (`UAAATeamComponent`, `IAAATeamAgent`) and use it; if either side has no team info, allow the hit.
- Replace `FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.Hit"))` with the native tag. Keep sending the same payload.
- Make the fallback-bounds warning use a member timestamp, not a function-static (multiple weapons).

## Acceptance (self-check by reading)
- LMB spam produces a chained combo (no dropped inputs, no double-stamina on chain? — stamina is charged **once per attack**, state it in the report), F produces a heavy.
- With an empty weapon montage array the procedural fallback path executes end-to-end and deals damage through `UAAAGE_Damage`.
- Every exit path of the ability removes timers/state/tracing. No raw `this` in timers/delegates that can outlive the ability instance.
- No existing public API removed. Update the header comments of changed classes. Report per `00_RULES.md`; list the three riskiest spots with file:line.

## Reviewer notes after P3 (Claude, authoritative)
- Native tags are now initialised in `FAAACoreModule::StartupModule` (previously **nothing** called `InitializeNativeGameplayTags`, so all tags were invalid). Do not add another init call.
- In `UAAAAttributeSet::PostGameplayEffectExecute` the combat component is found via the **avatar** actor (`Data.Target.GetAvatarActor()`) because for players the attribute set's owner is the PlayerState. Use avatar actors, never `GetOwningActor()`, to reach pawn components from GAS code.
- `UAAAAttributeComponent` (`Source/AAACharacter`) owns the death path (it listens to Health changes). Do not duplicate it; `State.Combat.Dead` is set by the attribute set, `State.Dead` by the attribute component.
