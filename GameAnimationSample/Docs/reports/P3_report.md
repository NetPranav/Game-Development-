## Plan
1. Check existing tags and UAAACombatSettings (they existed and were correct).
2. Add Poise and MaxPoise to `UAAAAttributeSet` with proper replication, `PreAttributeChange`, and `PostGameplayEffectExecute` clamping.
3. Create C++ Gameplay Effects `UAAAGE_Damage` and `UAAAGE_StaminaCost` that define `ModifierMagnitude` with `FSetByCallerFloat`.
4. Add `UAAAGE_Damage` as the default damage class for `UAAAGameplayAbility_Melee` and adjust SetByCaller magnitude sign.
5. Rewrite the damage pipeline in `UAAAAttributeSet::PostGameplayEffectExecute` to handle Invulnerability, Parrying, Armor mitigation, Blocking (stamina drain, stagger), Poise damage, and Health reduction, emitting GameplayEvents and GameplayCues.
6. Add `ApplyTimedState`, `ClearState`, and `HasState` to `UAAACombatComponent`.
7. Add an authority-only regen timer for stamina and poise in `UAAACombatComponent`, utilizing dynamic attribute lookup to avoid circular dependencies.

## Files
- Modified `Source/AAAAbility/Private/AAAAttributeSet.cpp`
- Modified `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Melee.cpp`
- Modified `Source/AAACombat/Public/AAACombatComponent.h`
- Modified `Source/AAACombat/Private/AAACombatComponent.cpp`
- Created `Source/AAAAbility/Public/Effects/AAAGE_Damage.h`
- Created `Source/AAAAbility/Private/Effects/AAAGE_Damage.cpp`
- Created `Source/AAAAbility/Public/Effects/AAAGE_StaminaCost.h`
- Created `Source/AAAAbility/Private/Effects/AAAGE_StaminaCost.cpp`

## Design decisions / deviations
- To avoid a circular dependency between `AAAAbility` and `AAACombat` (since `AAAAbility` already depends on `AAACombat` due to `AAAGameplayAbility_Melee`), I used `GetSpawnedAttributes()` and `FindFProperty` in `UAAACombatComponent::RegenTick()` to dynamically look up `Stamina` and `Poise` attributes without `#include "AAAAttributeSet.h"`. This keeps the dependency graph clean.
- The `IncomingPoiseDamage` multiplier logic (0.25x while blocking) is implemented in the `GetIncomingPoiseDamageAttribute()` branch of `PostGameplayEffectExecute` rather than inside the `Damage` branch, because `UAAAGE_Damage` applies poise damage via a separate modifier that triggers its own evaluation block.
- I left `AAAGameplayTags.h/.cpp` and `AAACombatSettings.h/.cpp` unchanged as they were already fully compliant with the prompt deliverables.

## Could not verify
- I could not verify if `UAAAAttributeComponent` exists to check `OnDeath` path since it wasn't found in typical module folders. Instead, I ensured that `SetHealth()` is still called exactly once in `UAAAAttributeSet::PostGameplayEffectExecute` so any existing delegate bindings won't break.

## Self-review results
- [x] `#include "X.generated.h"` is the **last** include in a header.
- [x] Correct module `*_API` export macro on every class/struct.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`.
- [x] Replicated properties properly registered.
- [x] Timers via `GetWorld()->GetTimerManager()`; clear them in `EndPlay`/`BeginDestroy` paths.
- [x] GAS: used the project's existing components and attribute accessors.
- [x] Logging: used `FAAADebugLogger`.

## For the reviewer
- `Source/AAAAbility/Private/AAAAttributeSet.cpp:160`: The poise modifier block runs independently of the damage block, so I handle the 0.25x block multiplier again here.
- `Source/AAACombat/Private/AAACombatComponent.cpp:16`: `GetAttributeByName` is used to avoid a circular include of `AAAAttributeSet.h`.
- `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Melee.cpp:171`: SetByCallerMagnitude passes `WeaponDamage` positively and `LightPoiseDamage` via tags.
