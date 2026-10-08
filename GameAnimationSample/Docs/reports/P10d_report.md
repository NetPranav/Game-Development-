## Plan
- Implement `RRHUD_DamageFx.cpp` to draw RE8-style damage effects: red vignette, blood splats, veins, desaturation.
- Modify `RRHUD.h` and `RRHUD.cpp` to remove player health bars from match HUD, replace them with the screen damage effects, add a posture bar, and add `rr.fx.hurt` and `rr.fx.lowhealth` debug commands.
- Modify `AAACombatComponent` to play montages (hit reactions, block, deflect) on the master AnimInstance using `FindAnimInstance()` to properly animate the player's Mover pawn.
- Fix knockback bypassing friction in `AAAMoverUtils::ApplyVelocityBurst` by ensuring all pawns (including `ACharacter` for testing) use the timer fallback or `LaunchCharacter` as appropriate, so knockback logic from `AAAKnockbackSubsystem` functions properly for both ACharacter and Mover.
- Reflect poise damage on deflect back to the attacker in `AAAAttributeSet::PostGameplayEffectExecute`.

## Files
- `Source/RRGame/Private/RRHUD_DamageFx.cpp`: Created. Handles damage VFX drawing on Canvas.
- `Source/RRGame/Public/RRHUD.h`: Modified to remove health properties and add Damage FX properties and `DebugInjectHit`.
- `Source/RRGame/Private/RRHUD.cpp`: Modified to call `DrawDamageFx()`, remove old health bar, draw posture bar, register console commands.
- `Source/AAACombat/Private/AAACombatComponent.cpp`: Implemented `FindAnimInstance()` and `PlayMontageOnOwner()`, and hooked up `OnBlockedEvent`, `OnStaggeredEvent`, `OnDeathEvent` to play C++ montages.
- `Source/AAAAbility/Private/AAAMoverUtils.cpp`: Restored `LaunchCharacter` check for `ACharacter` while letting Mover pawns use the `SetActorLocation` timer fallback.
- `Source/AAAAbility/Private/AAAAttributeSet.cpp`: Applied poise damage reflection when the defender is parrying.

## Design decisions / deviations
- For the blood vein streaks, a simple pseudo-random hash algorithm using `RotRect` was created to draw jaggy veins on the screen edges deterministically based on screen edge coordinates.
- For hit knockback, `AAAKnockbackSubsystem` already had the 115cm / 220cm / 55cm distance values present in the codebase. I updated `AAAMoverUtils` so Mover pawns use the timer correctly (bypassing time dilation), while `ACharacter` uses `LaunchCharacter`.
- I added a `PlayBlockImpact` function to `AAACombatComponent` to play the `DeflectMontage` on block.
- For parry VFX, I listed it below to configure the existing gameplay cue rather than hardcoding sparks.

## Could not verify
- The exact layout of the blood vein streaks on different aspect ratios since it uses standard UI scales without visual feedback, but it relies on normalized `U()` Canvas coords.

## Self-review results
- [x] `#include "X.generated.h"` is the last include. (N/A for new cpp files)
- [x] Correct module `*_API` export macro. (N/A for new files)
- [x] Every `UCLASS` constructor/override calls `Super::`.
- [x] Replicated properties use `DOREPLIFETIME`.
- [x] Pointers checked before use.
- [x] Timers cleared in `EndPlay`.
- [x] GAS tags used correctly.
- [x] Logging format correct.
- [x] `TArray` ranged-for non-mutating.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
- `Source/RRGame/Private/RRHUD_DamageFx.cpp`:100 - Ensure the hashing for vein streaks is deterministic enough across frames.
- `Source/AAAAbility/Private/AAAAttributeSet.cpp`:195 - Check the calculation of the reflected poise damage in the parry logic.
- `Source/AAACombat/Private/AAACombatComponent.cpp`:485 - Ensure `Multicast_OnHitConfirmed_Implementation` correctly plays hit reactions on clients.

**Editor Configuration Needed:**
- Set `DA_PlayerWeapon.Animations.DeflectMontage` and the `GameplayCue_Combat_Hit_Parried` blueprint to include spark/clang sound/VFX for parrying.
