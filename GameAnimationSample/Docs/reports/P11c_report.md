## Plan
- Add `BlueprintPure` getters to `ARRBoss` to expose boss state to the HUD (Segments remaining, fraction, posture fraction, staggered state, phase, and boss name).
- Add `State.Boss.Segmented` and `State.Boss.Perilous` as native tags in `AAAGameplayTags.cpp` so they are fully functional.
- Hook up `FRRBossEvents` to `ARRHUD` during `BeginPlay` and `EndPlay` to drive phase change and perilous attack UI presentations.
- Create `RRHUD_Boss.cpp` and implement `ARRHUD::DrawBossHud()` using `Canvas` primitives to match the Sekiro-style layout (side-by-side health segments with ghost trails, posture bar from centre, deathblow prompt, perilous warning glyph, phase transition shatter animation).
- Modify `RRHUD::DrawMatchHUD()` to skip drawing standard health bars for enemies that are `ARRBoss`.

## Files
- `Source/RRGame/Public/RRBoss.h` (modified)
- `Source/RRGame/Private/RRBoss.cpp` (modified)
- `Source/RRGame/Public/RRHUD.h` (modified)
- `Source/RRGame/Private/RRHUD.cpp` (modified)
- `Source/RRGame/Private/RRHUD_Boss.cpp` (created)
- `Source/AAACore/Public/AAAGameplayTags.h` (modified)
- `Source/AAACore/Private/AAAGameplayTags.cpp` (modified)

## Design decisions / deviations
- Handled the ghost trailing and shatter animation timings directly in `RRHUD_Boss.cpp`. It checks `BossGhostHealth` array against the target fractions each frame.
- Implemented `BossShatteredSegment` index mapping such that it always aligns with `RemSegs` tracking when health segments empty from right-to-left.

## Could not verify
- Could not visually test the stylized "Perilous glyph" (`危` style), but mapped it precisely out of rotated rects and lines per requirements.

## Self-review results
- [x] `#include "X.generated.h"` is the last include in a header.
- [x] Correct module `*_API` export macro on every class/struct/free function used across modules.
- [x] Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`.
- [x] Replicated properties use `DOREPLIFETIME`.
- [x] Delegates bound and unbound correctly (`BeginPlay` / `EndPlay`).
- [x] Pointers checked with `IsValid()` / null checks (`!Canvas`, `!Boss`).
- [x] Timers via `GetWorld()->GetTimerManager()`.
- [x] GAS native tags correctly added to `AAAGameplayTags`.
- [x] Logging categories respected.
- [x] FString/FName/FText conversions explicit.
- [x] Every new `.cpp` includes its own header first.

## For the reviewer
- `Source/RRGame/Private/RRHUD.cpp:382`: The check `!Enemy->IsA(ARRBoss::StaticClass())` requires `RRBoss.h` included in `RRHUD.cpp`, which I added.
- Blueprint/Assets needed: A `BP_RRBoss` child of `ARRBoss` cloned from `BP_RREnemy_Greystone` with mesh scale 1.2, and `AIControllerClass` set.
