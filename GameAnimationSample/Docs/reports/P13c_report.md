## Plan
Create an audio subsystem `URRAudioDirector` for `RRGame` that manages music states and combat SFX. I will create `RRAudioDirector.h` and `RRAudioDirector.cpp`, declare the class, bind to `FAAACombatEvents` and `FRRBossEvents`, handle the two audio components to crossfade music cleanly, and start the ambience wind on tick. Finally, I will add a console command to switch music states.

## Files
- Source/RRGame/Public/RRAudioDirector.h
- Source/RRGame/Private/RRAudioDirector.cpp

## Design decisions / deviations
- Used `UGameplayStatics::CreateSound2D` with TargetVolume parameter as 0 to start it faded out so the `FadeIn` interpolates from 0 cleanly, rather than setting it to Volume which might cause an initial pop if the component tries to start at target volume before fading in (though either might work). The `FadeIn` takes the target volume.
- Used `FMath::RandRange(0.92f, 1.08f)` for Pitch to randomize sword sounds.
- For `OnHitConfirmed` impact point being zero check, I used `Hit.ImpactPoint.IsNearlyZero()` and fallback to `Victim->GetActorLocation()` if needed.

## Could not verify
- The exact initial behavior of `FadeIn` if the component is created with `TargetVolume` vs `0.0f` as starting volume. Using `0.0f` to be safe as `FadeIn` takes care of the volume target. 

## Self-review results
- [x] `#include "X.generated.h"` is the **last** include in a header. Every type used in a `UPROPERTY/UFUNCTION` signature is included or forward-declared.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`.
- [x] Replicated properties: N/A
- [x] Delegates: dynamic multicast need `UFUNCTION()` handlers bound with `AddDynamic`. Lambdas capture `TWeakObjectPtr<>`, never raw `this` for timers/HTTP/async. (N/A, used native delegates)
- [x] Pointers: UObject members are `UPROPERTY()`-tracked (`TObjectPtr<>`/`TWeakObjectPtr<>`); check `IsValid()` before use.
- [x] Timers via `GetWorld()->GetTimerManager()`; clear them in `EndPlay`/`BeginDestroy` paths. (N/A)
- [x] GAS: use the project's existing `UAAAAbilitySystemComponent`, `UAAAAttributeSet`, `AAAGameplayTags`. (N/A)
- [x] Logging: define/declare a per-module log category if none exists.
- [x] FString/FName/FText conversions explicit; `TArray` ranged-for doesn't mutate the array; no `auto` copies of large structs in hot paths.
- [x] Every new `.cpp` includes its own header first. No unused includes of editor-only modules in runtime code.

## For the reviewer
- `RRAudioDirector.cpp:142`: The `FadingMusicComp` variable handles stopping/overriding correctly when toggling quickly between music tracks.
- `RRAudioDirector.cpp:211`: Fallback to `Victim->GetActorLocation()` when `Hit.ImpactPoint` is zero (like for non-physical hits/projectile hits without collision data).
- `RRAudioDirector.cpp:52`: Subscribing to native delegates using `AddUObject(this, ...)`.
