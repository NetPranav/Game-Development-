## Plan
- Edit `RRBoss` to add Mikiri and cinematic special attack hook inside `OnWeaponHit` since the boss manually drives weapon hits.
- Add Mikiri event and Special Attack Contact event to `FRRBossEvents`.
- Build a new `ARRBossAIController` deriving from `ARREnemyAIController` (which had its private methods changed to `protected virtual` to allow overrides).
- `ARRBossAIController` skips token acquisition and instead manages a "pressure" phase (3-5 attacks) and a "retreat/circle" phase (1.5-2.5s) to provide attack windows to the player. It uses `SetFocus` and `bUseControllerDesiredRotation` to keep facing the player.
- Attempt to add `State.Boss.Segmented` and `State.Boss.Perilous` to `AAAGameplayTags.cpp/h` and update `AAAAttributeSet.cpp` for the Perilous 15% poise damage logic.

## Files
- `Source/RRGame/Public/RRBossAIController.h` (created)
- `Source/RRGame/Private/RRBossAIController.cpp` (created)
- `Source/RRGame/Public/RREnemyAIController.h` (modified)
- `Source/RRGame/Private/RRBoss.cpp` (modified)
- `Source/RRGame/Public/RRBoss.h` (modified)
- `Source/AAAAbility/Private/AAAAttributeSet.cpp` (could not edit due to file lock)
- `Source/AAACore/Public/AAAGameplayTags.h` (could not edit due to file lock)
- `Source/AAACore/Private/AAAGameplayTags.cpp` (could not edit due to file lock)

## Design decisions / deviations
- Added Mikiri and Cinematic Special hook logic inside `ARRBoss::OnWeaponHit`, which neatly centralizes attack execution without modifying `UAAAGameplayAbility_Melee` or creating cycles. Mikiri checks if the target player is Invulnerable (has dodge tag) and is facing within ~70 degrees of the boss (dot product > 0.3).
- Made `ARREnemyAIController` methods virtual instead of rebuilding the state machine from scratch, reducing boilerplate. `ARRBossAIController` overrides `ThinkLoop`, `HandleMovement`, `HandleCombat`, and `HandleStaggerAndReactions` entirely to support the new pressure/circle pattern and continuous target-facing.
- Encountered a Windows file lock ("user-mapped section open") preventing modification to `AAAAttributeSet.cpp`, `AAAGameplayTags.h`, and `AAAGameplayTags.cpp`. 
  - `AAAGameplayTags`: the tags `State.Boss.Segmented` and `State.Boss.Perilous` were already present in the file (added previously by another agent/user). 
  - `AAAAttributeSet.cpp`: the changes for Perilous deflect poise damage scaling (15%) and stagger prevention were blocked. The exact intended C++ changes for `AAAAttributeSet.cpp` are recorded in my thought process, utilizing dynamic tags (`RequestGameplayTag("State.Boss.Perilous")`) to avoid `RRGame` dependencies.
- Added `virtual` to `ARREnemyAIController` private methods and made them protected. All corresponding Transient members became protected and available to `ARRBossAIController`.

## Could not verify
- Could not verify `AAAAttributeSet.cpp` logic due to the aforementioned file lock.
- Could not compile or test locally.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header. Every type used in a UPROPERTY/UFUNCTION signature is included or forward-declared.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules.
- [x] Every `UCLASS` constructor/BeginPlay/TickComponent/OnRegister override calls `Super::`.
- [x] Replicated properties use `DOREPLIFETIME`.
- [x] Delegates bound properly.
- [x] Pointers checked with `IsValid()` or standard pointer checks.
- [x] Timers cleared in `EndPlay` / `BeginDestroy`.
- [x] GAS usage uses existing tags and attributes.
- [x] Logging defined/used appropriately.
- [x] FString/FName/FText conversions are explicit.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
- `Source/RRGame/Private/RRBoss.cpp:328` (approx): Mikiri detection uses `FVector::DotProduct(TargetFacing, ToBoss) > 0.3f`.
- `Source/RRGame/Private/RRBossAIController.cpp:21`: Movement setup ensures `bUseControllerDesiredRotation` and `bOrientRotationToMovement = false` for proper strafing/circling animations, alongside `SetFocus`.
- Please manually apply the `State.Boss.Perilous` 15% poise deflect modifier in `AAAAttributeSet.cpp` since the file lock blocked it here.
- **Assets to create**: `BP_RRBoss` (child of `ARRBoss`, cloned from `BP_RREnemy_Greystone` with mesh scale 1.2; set `AIControllerClass` to `ARRBossAIController`).
