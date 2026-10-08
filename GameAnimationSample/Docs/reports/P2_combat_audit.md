# P2 — Combat Audit Report

## 1. Attack pipeline end-to-end
1. **Input**: `AAAAPlayerController::OnAttackAction` (or equivalent Lyra-style input binding via `FAAAAbilityInputBinding` in `AAACharacterSettings`) -> **[UNTESTED/MISSING in C++]**.
2. **Montage Playback**: Activated ability plays montage from `UAAAWeaponSettings::Animations.LightAttackCombos` -> **[UNTESTED/MISSING in C++]**.
3. **Trace Start**: `AnimNotifyState_WeaponTrace::NotifyBegin` (`Source/AAAAnimation/Private/Notifies/AnimNotifyState_WeaponTrace.cpp:11`) -> Gets `UAAACombatComponent` -> Calls `StartWeaponTrace()`.
4. **Trace Execution**: `AAAAWeaponActor::PerformTrace` (`Source/AAACombat/Private/AAAAWeaponActor.cpp:102`). Ticks every frame. **[BUG: Only sweeps the tip of the weapon (`TraceEndSocket`), ignoring the blade length]**.
5. **Hit Event**: `PerformTrace` sends `GameplayEvent.Combat.Hit` via `UAbilitySystemBlueprintLibrary::SendGameplayEventToActor` (`AAAAWeaponActor.cpp:192`).
6. **Damage Effect**: A GameplayEffect (triggered by the event) uses `UAAADamageExecutionCalc` -> **[UNTESTED/MISSING in C++]**.
7. **Calculation**: `UAAADamageExecutionCalc::Execute_Implementation` (`Source/AAAAbility/Private/AAADamageExecutionCalc.cpp:35`). Captures Base Damage and Armor. Calculates `MitigatedDamage = Damage * (100 / (100 + Armor))` and adds to `Damage` meta attribute.
8. **Attribute Change**: `UAAAAttributeSet::PostGameplayEffectExecute` subtracts `Damage` from `Health` -> **[STUBBED/ASSUMED]**.
9. **Death**: Health reaches 0, triggers death -> **[STUBBED/ASSUMED]**.

## 2. Gaps vs a good melee game
*   **Light combo chain with input buffer**: Partial (Data exists in `UAAAWeaponSettings`, input buffer logic missing).
*   **Heavy attack**: Partial (Data exists in `UAAAWeaponSettings`, activation logic missing).
*   **Block**: Missing.
*   **Parry**: Missing.
*   **Dodge i-frames**: Missing.
*   **Stamina cost+regen**: Partial (Cost data in weapon settings, `Stamina` attribute exists, regen missing).
*   **Poise/stagger**: Missing (No `Poise` attribute, only `State_Stunned` tags).
*   **Hit reactions (direction aware)**: Missing.
*   **Lock-on targeting**: Missing (C++ implementation absent).
*   **Hit-stop**: Missing.
*   **Camera shake**: Missing.
*   **Hit VFX/SFX hooks (GameplayCues)**: Partial (Tags exist e.g. `GameplayCue_Combat_Hit_Melee`, but hooks in trace are missing).
*   **Damage numbers**: Missing.
*   **Weapon draw/sheath**: Exists (`UAAACombatComponent::DrawWeapon` / `SheathWeapon`).
*   **Knockback**: Missing.
*   **Friendly-fire/team filtering**: Partial (`UAAATeamComponent` exists, but `AAAAWeaponActor::PerformTrace` lacks team filtering checks).

## 3. Attributes
Attributes in `UAAAAttributeSet`:
*   `Health` (Replicated, Clamps in PreAttributeChange assumed)
*   `MaxHealth` (Replicated)
*   `Stamina` (Replicated)
*   `MaxStamina` (Replicated)
*   `Armor` (Replicated)
*   `Damage` (Meta attribute, Not Replicated)
*   `MovementSpeed` (Replicated)
*   `Experience` (Replicated)
*   `Level` (Replicated)
*   *Missing*: Poise, AttackPower (Damage is driven by weapon base stats and GE magnitudes).
*   *Initialization*: By `UAAAAbilitySystemComponent::GrantStartupData` using defaults from `UAAACharacterSettings::Vitals`.

## 4. Data assets
*   **`UAAAWeaponSettings`**:
    *   Fields: `WeaponType`, `Visuals` (Static/Skeletal mesh, sockets), `Combat` (BaseDamage, AttackSpeedMultiplier, Range, StaminaCost, Weight), `Abilities` (GrantedAbilities, GrantedEffects, Tags), `Animations` (Light/Heavy combos, Equip/Unequip).
    *   Read by code: `Visuals`, `Abilities` (in `UAAACombatComponent::EquipWeapon`), `Animations.Equip/Unequip` (in `DrawWeapon`/`SheathWeapon`).
    *   Dead fields: `Combat` stats and `Light/HeavyAttackCombos` arrays are not read directly by C++ code (expected to be read by GAS abilities).
*   **`UAAACharacterSettings`**:
    *   Fields: `Vitals`, `Movement`, `Abilities`, `Camera`, `Input` (Lyra-style tag bindings).
*   **`UAAAItemWeaponSettings`**: Missing / Not found.

## 5. Gameplay tags
Relevant tags in `AAAGameplayTags`:
*   *State*: `State_Combat`, `State_Combat_Drawn`, `State_Combat_Holstered`, `State_Combat_Unarmed`, `State_Combat_Invulnerable`
*   *Damage*: `Damage_Physical`, `Damage_Fire`, etc.
*   *Input*: `InputTag_Attack`, `InputTag_SecondaryAttack`, `InputTag_Dodge`, `InputTag_DrawSheath`
*   *Cues*: `GameplayCue_Combat_Hit_Melee`, `GameplayCue_Combat_Hit_Sword`

## 6. Movement of pawns
*   **Player Pawn**: Moves via Epic's Mover plugin (`UMoverComponent`), driven by `BP_SandboxCharacter` and Lyra-style input mappings.
*   **AI Controlled (`AAAAATestEnemy`)**: Should reuse the Epic Game Animation Sample's `UMoverComponent` configuration (e.g., a child of `BP_SandboxCharacter_Mover`) controlled by `AAAAAIController`.

## 7. Montage/animation contract
*   **Expected Notifies**: `AnimNotifyState_WeaponTrace` is expected on attack montages to start/stop tracing.
*   **Expected Slots**: C++ expects `DrawWeapon` / `SheathWeapon` to play `EquipAnimation` and `UnequipAnimation` on default slots.
*   **AnimBP Requirements**: The AnimBP must react to `State_Combat_Drawn` (or `IsWeaponDrawn()`) to transition into combat idles and locomotion.

## 8. Replication & prediction
*   **Server RPCs**: `EquipWeapon` enforces `HasAuthority()`, making weapon swapping server-authoritative.
*   **Predicted**: GAS abilities (attacks, dodges) are predicted.
*   **Desync Risks**: `StartWeaponTrace` enables ticking `PerformTrace` which fires `GameplayEvent.Combat.Hit`. If this trace runs on autonomous proxy clients without robust target validation, it introduces severe desync and cheating risks.

## 9. Compile-health
*   `AAAAWeaponActor::PerformTrace` (`AAAAWeaponActor.cpp:114`) — **Severity: High (Logic Bug)**. The trace logic only sphere-traces from the previous frame's `TraceEndSocket` to the current `TraceEndSocket`. It ignores `TraceStartSocket` entirely for the sweep, missing any hits along the blade.
*   `UAAACombatComponent::DrawWeapon` (`AAACombatComponent.cpp:111`) — **Severity: Low**. Hardcoded fallback path `/Game/AAA/DataAssets/Asset/Sword/DA_TestSword` could fail in packaged builds.

## 10. Recommended extension points
*   `AAAAWeaponActor::PerformTrace`: Rewrite to use a Box Trace or a sweep from `TraceStartSocket` to `TraceEndSocket` that interpolates between previous frame transforms.
*   `UAAATeamComponent`: Add interface calls in `PerformTrace` to ignore friendly fire before sending hit events.
*   `UAAAAttributeSet`: Add a `Poise` attribute to handle stagger thresholds.
*   `UAAACombatComponent`: Needs a combo-manager struct or component to handle the input buffer and index tracking for `LightAttackCombos`.

## Proposed implementation order
1. Fix `AAAAWeaponActor::PerformTrace` to correctly trace the weapon's blade volume.
2. Implement Team/Faction filtering in the weapon trace.
3. Build `UAAAComboManagerComponent` for input buffering.
4. Implement the GAS `UGameplayAbility_MeleeAttack` to play montages and consume stamina.
5. Add `Poise` and Stagger logic to `UAAAAttributeSet`.
6. Implement Hit Reactions and Directional Hit logic via `GameplayCues`.
7. Add Block and Parry GAS abilities with corresponding Tags.
8. Implement Dodge with i-frames (`State_Combat_Invulnerable`).
9. Add Lock-on targeting in the Player Controller.
10. Refine replication for weapon traces (Server-side validation).
