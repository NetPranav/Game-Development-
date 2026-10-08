# P6c — Lock-on targeting in the player controller (module AAAGame)
Read first: `Docs/prompts/00_RULES.md`, `Docs/reports/MODULE_DEPS.md`, `Docs/reports/FILE_INDEX.md`, `Docs/reports/P5_report.md`. **Never call run_command or any shell tool — a denied shell call ends your session. No directory listing is needed; all paths are named.** Spec: section 4 of `Docs/prompts/P6_reactions_feedback_lockon.md` plus its "Reviewer decisions" section (they override).

## Files
- EDIT `Source/AAAGame/Public/GameFramework/AAAAPlayerController.h`
- EDIT `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`
- EDIT `Source/AAAGame/AAAGame.Build.cs` only if an *engine* module is needed (verify). AAAGame must NOT depend on AAACharacter, AAAAnimation or RRGame.
Read both controller files fully first (they already bind R, LMB, F, RMB and LShift via `BindKey`; P5 added debug console commands there).

## Do exactly this
- `ToggleLockOn()` bound to **Q** and **MiddleMouseButton**; `CycleLockOn(int32 Dir)` on **LeftBracket / RightBracket** (Tab is the menu key).
- Candidates: actors within `UAAACombatSettings::LockOnRange` of the pawn that implement `IAbilitySystemInterface` (or have an ASC), are not dead (`State.Combat.Dead` checked on the ASC), are not on the same team (use `IGenericTeamAgentInterface` / `FGenericTeamId` of the controller and the target; allow when team info is missing), lie within `LockOnMaxAngleDegrees` of the camera forward, and are visible (line trace from the pawn eye to the target chest on ECC_Visibility ignoring both). Score = angle + distance. Auto-break when the target dies, exceeds range x1.3, or line of sight is lost for more than 1.5 s.
- While locked, in `PlayerTick` smoothly rotate `ControlRotation` toward the target chest (`FMath::RInterpTo`, speed about 8, pitch clamped -35..+25). A large look-input flick (more than 30 degrees away for 0.4 s) releases the lock. Do not touch Mover or pawn facing; this is a camera-only lock (say so in the report).
- Expose `UFUNCTION(BlueprintPure) AActor* GetLockedTarget() const` and `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockTargetChanged, AActor*, NewTarget)` as `OnLockTargetChanged` (BlueprintAssignable). Clear timers and state on `EndPlay` / `OnUnPossess`.
- Do not break the existing menu/pause logic; keep every existing public function and binding.

Finish: write `Docs/reports/P6c_report.md` (format in `00_RULES.md`, honest self-review ticks) and append a line `P6c done` to `Docs/PROGRESS.md`.
