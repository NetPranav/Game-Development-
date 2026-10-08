## Plan
1. Implement `UAAAMoverUtils` BlueprintFunctionLibrary with Mover integration (`ApplyVelocityBurst`, `GetPlanarVelocity`, `GetMoveIntent`).
2. Add Mover plugin dependency to the correct module.
3. Add Block and Parry states to `UAAACombatComponent` with server validation, animation montage hooks, and stamina auto-release via tag listening.
4. Bind generic gameplay event for `GameplayEvent.Combat.Parried` to apply `State_Combat_Staggered` to the attacker.
5. Add `BlockMontage` and `ParryMontage` to `UAAAWeaponSettings`.
6. Bind RMB inputs to `SetBlocking(bool)` in `AAAAAPlayerController`.
7. Rewrite `UAAAGameplayAbility_Dodge` to rely entirely on `UAAAMoverUtils` instead of `ACharacter` root motion, support accurate stamina cost deduction via SetByCaller, and configure exact timer-based i-frames for the middle of the dodge.
8. Add debug console commands to apply damage and stagger.

## Files
- Modified `Source/AAACharacter/AAACharacter.Build.cs` (cleared old attempts)
- Modified `Source/AAAAbility/AAAAbility.Build.cs` (Added Mover)
- Created `Source/AAAAbility/Public/AAAMoverUtils.h`
- Created `Source/AAAAbility/Private/AAAMoverUtils.cpp`
- Modified `Source/AAACombat/Public/DataAssets/AAAWeaponSettings.h`
- Modified `Source/AAACombat/Public/AAACombatComponent.h`
- Modified `Source/AAACombat/Private/AAACombatComponent.cpp`
- Modified `Source/AAAGame/Public/GameFramework/AAAAPlayerController.h`
- Modified `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`
- Modified `Source/AAAAbility/Public/Abilities/AAAGameplayAbility_Dodge.h`
- Modified `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Dodge.cpp`
- (Deleted copies) `Source/AAACharacter/Public/AAAMoverUtils.h`, `Source/AAACharacter/Private/AAAMoverUtils.cpp`

## Design decisions / deviations
- Moved `UAAAMoverUtils` from `AAACharacter` to `AAAAbility` because `AAACharacter` depends on `AAAAbility`, and `AAAAbility` needs to use `UAAAMoverUtils` for the Dodge ability. Putting it in `AAACharacter` would cause a circular dependency module cycle (or force Dodge to not use it).
- Placed the debug console commands in `AAAAAPlayerController.cpp` instead of `UAAACombatComponent` because `UAAACombatComponent` does not (and cannot) depend on `AAAAbility` (which contains `UAAAGE_Damage`), whereas `AAAGame` depends on both and makes a natural location for player-invoked debug commands.
- Used `FApplyVelocityEffect` via `QueueInstantMovementEffect` from the Experimental Mover plugin.

## Could not verify
- Whether the Mover plugin expects `bAdditiveVelocity` to be strictly false for dodge impulses in this project, assuming false to provide immediate launch behavior overriding current velocity.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header.
- [x] Correct module `*_API` export macro on every class/struct/free function.
- [x] Every `UCLASS` constructor/BeginPlay... overrides call `Super::`.
- [x] Replicated properties properly setup.
- [x] Delegates use correct bindings.
- [x] Pointers checked.
- [x] Timers cleared.
- [x] GAS utilized correctly.
- [x] FString/FName conversions explicit.
- [x] No unused includes.

## For the reviewer
1. `Source/AAAAbility/Private/AAAMoverUtils.cpp:41` - The fallback kinematic slide uses a timer lambda capturing weak pointers to the actor and world, relying on the lambda terminating when time exceeds.
2. `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp:177` - The debug commands create default objects for `UAAAGE_Damage`, expecting it to exist and be fully configured in CDO.
3. `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Dodge.cpp:188` - I-frames are handled by separate timers that cancel each other appropriately in `EndAbility`, ensuring safe state cleanup regardless of interruption.
