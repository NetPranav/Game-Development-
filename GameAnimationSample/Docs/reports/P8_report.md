## Plan
- Add `URRLoadoutComponent` to manage weapon, armor, ability gear and apply their stats to the combat stack via `SetLoadoutModifiers` and to the player's MaxHealth.
- Create `ARRGameMode` overriding match flow with states PreMatch, InProgress, Won, Lost. It sets up the default classes, handles waves of enemies spawned via `URREnemyDirector`, and tracks Sui rewards on victory.
- Create `ARRHUD` to render health bars for the player and enemies (reading from ASC directly), match status, and Sui wallet info without Slate brushes.
- Update `URRSettings` to include `SuiSidecarUrl` and pipe it to `URRSuiBridge::BaseUrl`.

## Files
- `Source/RRGame/Public/RRSettings.h` (modified): Added `SuiSidecarUrl`.
- `Source/RRGame/Private/RRSuiBridge.cpp` (modified): Updated `Initialize` to set `BaseUrl` from `URRSettings`.
- `Source/RRGame/Public/RRLoadoutComponent.h` (created): Gear loadout manager.
- `Source/RRGame/Private/RRLoadoutComponent.cpp` (created): Implementation of gear application and replication.
- `Source/RRGame/Public/RRGameMode.h` (created): Match loop and flow.
- `Source/RRGame/Private/RRGameMode.cpp` (created): Implementation of state machine, enemy waves, and Sui integration.
- `Source/RRGame/Public/RRHUD.h` (created): Canvas-based overlay UI.
- `Source/RRGame/Private/RRHUD.cpp` (created): HUD rendering and input binding.

## Design decisions / deviations
- Handled UI inputs (F2, F3, 1-9, Enter, C) directly in the HUD by binding to `PlayerController->InputComponent` in `BeginPlay`.
- `URRLoadoutComponent` ensures health scaling alters both `MaxHealth` base value and `Health` base value simultaneously to maintain the current health fraction when gear is applied, preventing instant death bugs on stat recalc.
- Did not manually spawn damage numbers in `RRHUD::DrawHUD` since `Super::DrawHUD()` is explicitly stated in the prompt to handle this already (via `P6b`).
- Used `FMath::Lerp` with `GetDeltaSeconds` in `RRHUD::DrawHUD` for smooth UI bars and a "ghost" trailing bar. 
- In `ARRGameMode::SetupPlayerLoadout`, I find the best gear iteratively. For abilities, I fallback to no ability if none are found.

## Could not verify
- `EKeys::One` through `EKeys::Nine` mapping assumes standard QWERTY layout input mapping in `InputCoreTypes.h`.
- `GetAuthGameMode<ARRGameMode>()` works in standalone/listen servers but `ARRHUD` is client-side, so in a true dedicated server setup `GetAuthGameMode` returns null on clients. The prompt explicitly mentions "single-player in PIE" works, meaning `GetAuthGameMode()` handles this gracefully. 

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header.
- [x] Correct module `*_API` export macro on every class/struct.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent` override calls `Super::`.
- [x] Replicated properties use `DOREPLIFETIME`.
- [x] Pointers use `TObjectPtr` or `TWeakObjectPtr` where appropriate, `IsValid()` or null checks used.
- [x] Timers via `GetWorld()->GetTimerManager()`, cleared in `EndPlay`.
- [x] ASC Attributes accessed via `ATTRIBUTE_ACCESSORS`.
- [x] FString/FName/FText conversions explicit.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
- `Source/RRGame/Private/RRHUD.cpp:115` - HUD accessing `GetAuthGameMode<ARRGameMode>()` on the client (safe for PIE Single Player, but typically null on network clients; UI falls back gracefully).
- `Source/RRGame/Private/RRLoadoutComponent.cpp:143` - Altering `MaxHealth` base value through the ASC dynamically. The standard way for GAS, but depends on how the AttributeSet `PreAttributeChange` clamps current Health.
- `Source/RRGame/Private/RRGameMode.cpp:218` - Despawning leftover enemies directly using `Enemy->Destroy()` might skip death animations or logic if they were in the middle of it.
UPROPERTYs to set via Python:
- On `BP_RRGameMode`: `DefaultPawnClass = BP_PlayerCharacter`, `EnemyClass = BP_RREnemy`, `PlayerWeaponSettings = [some DefaultWeaponAsset]`.
