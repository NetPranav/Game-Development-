## Plan
Created module `RRGame` to hold the Sui bridge and gear types. 
- Created `RRGame.Build.cs`, `RRGame.h`, `RRGame.cpp` following the module pattern.
- Updated `GameAnimationSample.uproject`, `GameAnimationSample.Target.cs`, and `GameAnimationSampleEditor.Target.cs` to include `RRGame` module.
- Created `RRTypes.h` containing `ERRGearKind`, `FRRGearData`, and `FRRRewardResult` as `BlueprintType` with a static `FromJson` helper.
- Created `URRSuiBridge` as a `UGameInstanceSubsystem` handling HTTP requests. Contains `SendRequest` helper, `CheckHealth`, `BeginWalletConnect`, `FetchGear`, `RequestReward`, `UpgradeWeapon`. Wallet connection polling uses `FTimerManager`.
- Added the `rr.sui.test` console command.

## Files
- `GameAnimationSample.uproject` (modified)
- `Source/GameAnimationSample.Target.cs` (modified)
- `Source/GameAnimationSampleEditor.Target.cs` (modified)
- `Source/RRGame/RRGame.Build.cs` (added)
- `Source/RRGame/Public/RRGame.h` (added)
- `Source/RRGame/Private/RRGame.cpp` (added)
- `Source/RRGame/Public/RRTypes.h` (added)
- `Source/RRGame/Public/RRSuiBridge.h` (added)
- `Source/RRGame/Private/RRSuiBridge.cpp` (added)

## Design decisions / deviations
- Used `FConsoleCommandWithWorldArgsDelegate` in `IConsoleManager` to register the test command natively, rather than `FAutoConsoleCommand`, giving cleaner access to `GetWorld()` for the timer.

## Could not verify
- Test environment execution / actual Sui network requests as the compiler / UBT were not executed per the rules.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in headers.
- [x] Correct module `*_API` export macro.
- [x] UCLASS overrides call `Super::`.
- [x] Delegates bound properly and capture weak this.
- [x] Timers managed properly and cleared in Deinitialize.
- [x] Own header included first in cpp.

## For the reviewer
- `Source/RRGame/Private/RRSuiBridge.cpp:115` - JSON parsing array deserialization loop for `OwnedGear`.
- `Source/RRGame/Private/RRSuiBridge.cpp:166` - Ensure the timer `CancelWalletConnect()` clears gracefully when `GetWorld()` might be null.
- `Source/RRGame/Private/RRSuiBridge.cpp:211` - Reward json parsing `minted` sub-object logic checks `MintedObj->IsValid()`.
