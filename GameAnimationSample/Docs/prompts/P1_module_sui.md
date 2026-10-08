# P1 — `RRGame` module scaffold + Sui bridge
PROMPT_ID = `P1`. First read `Docs/prompts/00_RULES.md`, `Docs/PLAN.md`, `Docs/SPEC_SUI.md`, and task **T1** in `Docs/SPEC_CPP.md`.

## Context you must verify by reading (do not assume)
- `Source/AAAAI/AAAAI.Build.cs`, `Source/AAAAI/Public/AAAAI.h`, `Private/AAAAI.cpp` → copy exactly how an existing module is declared (module class, `IMPLEMENT_MODULE`, log category).
- `GameAnimationSample.uproject` (Modules array) and `Source/GameAnimationSample.Target.cs`, `Source/GameAnimationSampleEditor.Target.cs` → register `RRGame` the same way the other modules are (`Runtime`, `PreDefault`, in both Target files' `ExtraModuleNames`). Keep JSON valid, tabs preserved.
- The sidecar source `../Sui/sidecar/server.js` (repo root `Sui/`) is the **ground truth** for the HTTP contract: field names, `seed` as string, `{ok:false,error}` errors.

## Deliverables
1. Module `Source/RRGame/` with `RRGame.Build.cs` (deps in SPEC_CPP T1), `Public/RRGame.h`, `Private/RRGame.cpp`, log category `LogRR` (`RRGAME_API`).
2. `Public/RRTypes.h`: `ERRGearKind`, `FRRGearData`, `FRRRewardResult` (all `BlueprintType`, with `UPROPERTY(BlueprintReadWrite)`), plus static helper `FRRGearData::FromJson(const TSharedPtr<FJsonObject>&)` (and `ToDisplayString()` e.g. `"Pulse Blade  DMG 25  Lv1"`). Seed parsed with `FCString::Atoi64`; tolerate missing fields with defaults — never crash on malformed JSON.
3. `URRSuiBridge` exactly per SPEC_CPP T1 with these quality requirements:
   - Single private helper `SendRequest(Verb, Path, OptionalBodyJson, TFunction<void(bool bOk, TSharedPtr<FJsonObject>)>)` so each endpoint is ~10 lines. 15 s timeout (`SetTimeout`). Treat `ok:false` or non-2xx as failure and surface `error` text via `OnBridgeError`.
   - Wallet polling: `FTimerHandle`, interval 1 s, max 120 polls, stops on success/cancel/`Deinitialize`; expose `CancelWalletConnect()`.
   - All callbacks marshal back safely (weak self; HTTP callbacks already run on the game thread but still check `IsValid(this)`).
   - Reward result parsing: `url`, `digest`, `creditsAdded`, optional `minted` Gear.
   - `rr.sui.test` console command registered via `FAutoConsoleCommandWithWorldAndArgs` (or `IConsoleManager`), logging results with `LogRR`.
   - Offline-friendly: if the sidecar is down, `OnBridgeError("Sui sidecar unreachable (is it running on :3001?)")` once, no spam, game continues.
4. No UI, no gameplay in this prompt.

## Acceptance (you self-check by reading)
- Every HTTP path/field in `SPEC_SUI.md` has a code path; `/upgrade` implemented but handles the 501 error gracefully.
- uproject + both Target.cs contain `RRGame`; JSON/C# syntax valid.
- `Build.cs` private deps do not create a circular dependency (RRGame depends on AAA* modules, never the reverse).
Then follow the working method and report per `00_RULES.md`.
