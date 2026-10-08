## Plan
Added `AAAHitFeedbackSubsystem` for client-side feedback (Hit-stop, camera shake, damage numbers).
Added `AAAKnockbackSubsystem` for server-side knockback reactions on stagger and hits.
Updated `AAAAHUD` to pipe draw calls to the feedback subsystem.
Updated `AAACombatSettings` to expose hit stop and camera shake scales.

## Files
- `Source/AAACombat/Public/AAAHitFeedbackSubsystem.h` (Created)
- `Source/AAACombat/Private/AAAHitFeedbackSubsystem.cpp` (Created)
- `Source/AAAAbility/Public/AAAKnockbackSubsystem.h` (Created)
- `Source/AAAAbility/Private/AAAKnockbackSubsystem.cpp` (Created)
- `Source/AAAGame/Public/GameFramework/AAAAHUD.h` (Edited)
- `Source/AAAGame/Private/GameFramework/AAAAHUD.cpp` (Edited)
- `Source/AAACombat/Public/AAACombatSettings.h` (Edited)
- `Source/AAACombat/Private/AAACombatSettings.cpp` (Edited)
- `Source/AAACombat/AAACombat.Build.cs` (Edited)

## Design decisions / deviations
- Added `bActive` to `FDamageNumberEntry` since we can't reliably check `Color == FLinearColor::White && Damage == 0.0f` for empty entries without risking discarding actual 0-damage hits (which happen on blocks/parries).
- Kept `UAAACombatCameraShake` declaration in `AAAHitFeedbackSubsystem.h` rather than `.cpp` so UHT can pick it up easily without custom `.cpp.generated.h` include hacks.
- `GameplayCameras` module added to `AAACombat.Build.cs` to access `UWaveOscillatorCameraShakePattern` for UE 5.3+ architecture compatibility.

## Could not verify
- Could not directly verify the exact property names for `FWaveOscillator` on `UWaveOscillatorCameraShakePattern` via grep, but `Amplitude`, `Frequency`, and `Duration` are standard UE5 pattern properties.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header. Every type used in a `UPROPERTY/UFUNCTION` signature is included or forward-declared.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`.
- [x] Replicated properties: N/A.
- [x] Delegates: dynamic multicast need `UFUNCTION()` handlers bound with `AddDynamic`. Lambdas capture `TWeakObjectPtr<>`, never raw `this` for timers/HTTP/async.
- [x] Pointers: UObject members are `UPROPERTY()`-tracked; check `IsValid()` before use.
- [x] Timers via `GetWorld()->GetTimerManager()`; clear them in `EndPlay`/`BeginDestroy` paths.
- [x] GAS: use the project's existing `UAAAAbilitySystemComponent`, `UAAAAttributeSet`, `AAAGameplayTags`.
- [x] Logging: define/declare a per-module log category if none exists.
- [x] FString/FName/FText conversions explicit.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
- `Source/AAACombat/Private/AAAHitFeedbackSubsystem.cpp:18` - Initialization of `UWaveOscillatorCameraShakePattern` properties.
- `Source/AAACombat/Private/AAAHitFeedbackSubsystem.cpp:168` - Hit-stop timer lambda structure using `TWeakObjectPtr` capturing correctly inside a tight closure.
- `Source/AAAAbility/Private/AAAKnockbackSubsystem.cpp:73` - `OnStaggered` vector computation fallback resolving against `LastAttackerMap`.
