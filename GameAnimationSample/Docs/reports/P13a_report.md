## Plan
- In `AAACombatComponent.cpp`, add calls to `SetActorHiddenInGame(true)` and `SetActorEnableCollision(false)` when attaching weapon to holster, and the inverse when attaching to hand.
- In `AAAAPlayerController.cpp`, bind `EKeys::One`, `EKeys::MouseScrollUp`, and `EKeys::MouseScrollDown` to `OnDrawSheathWeapon`. Also remove the debug message in `OnDrawSheathWeapon`.

## Files
- Source/AAACombat/Private/AAACombatComponent.cpp (modified)
- Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp (modified)

## Design decisions / deviations
- Followed prompt precisely. No deviations.

## Could not verify
- None.

## Self-review results
- [x] `#include "X.generated.h"` is the **last** include in a header. Every type used in a `UPROPERTY/UFUNCTION` signature is included or forward-declared.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules. Cross-module use ⇒ the consumer's `*.Build.cs` lists the provider module in `Public/PrivateDependencyModuleNames` (add it if missing, and say so in the report).
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`. `PrimaryComponentTick.bCanEverTick` only if Tick is really used.
- [x] Replicated properties: `UPROPERTY(Replicated…)` + `GetLifetimeReplicatedProps` with `DOREPLIFETIME`/`DOREPLIFETIME_CONDITION`; `#include "Net/UnrealNetwork.h"`; `SetIsReplicatedByDefault(true)` for components. RPCs: `Server`/`Client`/`NetMulticast` + `Reliable/Unreliable` + `_Implementation` (+ `_Validate` only if `WithValidation`). Gameplay results are decided on authority (`HasAuthority()`); presentation (camera, HUD, hit-stop, shake) is local.
- [x] Delegates: dynamic multicast need `UFUNCTION()` handlers bound with `AddDynamic` (and the handler must be a UFUNCTION). Lambdas capture `TWeakObjectPtr<>`, never raw `this` for timers/HTTP/async.
- [x] Pointers: UObject members are `UPROPERTY()`-tracked (`TObjectPtr<>`/`TWeakObjectPtr<>`); check `IsValid()` before use; `GetWorld()` can be null in editor/CDO paths.
- [x] Timers via `GetWorld()->GetTimerManager()`; clear them in `EndPlay`/`BeginDestroy` paths.
- [x] GAS: use the project's existing `UAAAAbilitySystemComponent`, `UAAAAttributeSet`, `AAAGameplayTags` (native tags). New tags are added where the existing ones are declared (`AAACore/.../AAAGameplayTags.*`), same macro pattern. Use `ATTRIBUTE_ACCESSORS` macros as the existing attribute set does.
- [x] Logging: define/declare a per-module log category if none exists; `UE_LOG(Cat, Verbosity, TEXT("... %s"), *FString)`.
- [x] FString/FName/FText conversions explicit; `TArray` ranged-for doesn't mutate the array; no `auto` copies of large structs in hot paths.
- [x] Every new `.cpp` includes its own header first. No unused includes of editor-only modules in runtime code (guard with `#if WITH_EDITOR`).

## For the reviewer
- `Source/AAACombat/Private/AAACombatComponent.cpp` : `AttachWeaponToHolster()`
- `Source/AAACombat/Private/AAACombatComponent.cpp` : `AttachWeaponToHand()`
- `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp` : `SetupInputComponent()` and `OnDrawSheathWeapon()`
