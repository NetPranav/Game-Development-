## Plan
1. Make `AAAAAHUD::ToggleGameMenu` virtual, then override it in `ARRHUD` to just set `bInventoryOpen` and input modes, returning the open state (no UMG widget spawned).
2. Add `CloseMenuIfOpen()` to `AAAAAPlayerController` and bind the `Escape` key to it, ensuring `bExecuteWhenPaused = true`.
3. Add `FRRInventoryItem` to `RRLoadoutComponent.h` and implement `BuildInventory()` to merge the hardcoded local items and on-chain gear from the Sui bridge.
4. Add `SwordMeshes` to `URRSettings` to resolve paths, and implement weapon visual swapping inside `URRLoadoutComponent::Equip`. 
5. Implement a robust `DrawInventory` inside `ARRHUD`, drawing everything onto the Canvas and polling mouse input manually (`IsInputKeyDown(EKeys::LeftMouseButton)`) to ensure UI clicks work flawlessly while the game is paused.
6. Disable in-match panels when `bInventoryOpen` is true. Let digits 1-9 equip the items from the combined inventory by modifying `EquipGearIndex`.

## Files
- `Source/AAAGame/Public/GameFramework/AAAAHUD.h`
- `Source/AAAGame/Public/GameFramework/AAAAPlayerController.h`
- `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`
- `Source/RRGame/Public/RRHUD.h`
- `Source/RRGame/Private/RRHUD.cpp`
- `Source/RRGame/Public/RRLoadoutComponent.h`
- `Source/RRGame/Private/RRLoadoutComponent.cpp`
- `Source/RRGame/Public/RRSettings.h`
- `Source/RRGame/Private/RRSettings.cpp`

## Design decisions / deviations
- For mouse interactions while the game is paused, I opted to poll `PC->GetMousePosition` and `PC->IsInputKeyDown` along with a previous-frame boolean tracker manually instead of using `AHUD::AddHitBox`. This bypasses any engine quirks where hitboxes might drop clicks if the action system isn't ticking the same way during a pause, ensuring responsive UI clicks.
- I set `bExecuteWhenPaused = true` on the F2, F3, and digit key bindings so they can actually work while the menu is open, and removed the `if (bInventoryOpen) return;` block on `EquipGearIndex` so 1-9 keys function as requested.
- I chose to put `BuildInventory` directly on `URRLoadoutComponent` since it aggregates local gear with the Sui bridge gear.

## Could not verify
- Engine exact hitbox behavior while paused; manual mouse-tracking was implemented to be completely certain.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header.
- [x] Every type used in a `UPROPERTY/UFUNCTION` signature is included or forward-declared.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`.
- [x] Replicated properties: `UPROPERTY(Replicated…)` + `GetLifetimeReplicatedProps`.
- [x] Pointers: UObject members are `UPROPERTY()`-tracked (`TObjectPtr<>`/`TWeakObjectPtr<>`); check `IsValid()` before use.
- [x] Timers via `GetWorld()->GetTimerManager()`.
- [x] GAS tags usage.
- [x] Logging categories.
- [x] FString/FName/FText conversions explicit.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
1. `Source/RRGame/Private/RRHUD.cpp:392` (Manual mouse polling logic using `IsInputKeyDown` to guarantee click detection while paused).
2. `Source/RRGame/Private/RRLoadoutComponent.cpp:75` (Duplication of `UAAAWeaponSettings` and direct injection of `Visuals.StaticMesh` based on visual variant).
3. `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp:78` (Binding Escape to `CloseMenuIfOpen` with `bExecuteWhenPaused = true` natively in the component).

**Important configuration notes:**
`BP_RRHUD.GameMenuClass` configuration does not matter anymore because `ARRHUD` overrides `ToggleGameMenu()` to avoid checking it or spawning a UMG widget entirely.
