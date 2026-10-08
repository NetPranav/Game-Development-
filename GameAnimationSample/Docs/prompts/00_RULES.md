# Shared rules — READ FIRST, apply to every prompt

You are implementing C++ gameplay for an Unreal Engine **5.8** project (`GameAnimationSample.uproject`, repo root = `A:\UNREAL PROJECT\Game`, project dir = `GameAnimationSample/`). Engine source is on disk at `A:\UNREAL\UE_5.8\Engine\Source` and `...\Engine\Plugins` — **grep it to confirm any engine API signature you are not 100% sure of. Never invent an API.** A reviewer (Claude) will read your diff and report before anything is compiled, so correctness-by-inspection matters more than speed. Quality over speed; take as many steps as needed.

## Working method (mandatory order)
1. **Read** the design docs named in your prompt plus every existing file you will touch or call. Use grep to find real names. Existing code is authoritative; adapt to it, do not fight it.
2. **Plan briefly** in `Docs/reports/<PROMPT_ID>_report.md` (what you will add/change and why) *before* coding.
3. **Implement** in small, complete files. No TODO stubs, no commented-out code, no placeholder returns.
4. **Self-review pass** (separate step, not optional): re-open every file you created/changed and check the list below. Fix what you find.
5. **Finish the report**: files created/changed (one line each), any deviation from the prompt and why, anything you could not verify, open questions. Then append `<PROMPT_ID> done: <one line>` to `Docs/PROGRESS.md` under "Log".
6. **Do NOT compile, do NOT run the editor/UBT, do NOT launch builds.** Do not stop at asking questions: make the most reasonable choice, record it in the report.

## Hard constraints
- Never edit or create: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `Content/**/*.uasset|*.umap` (binary assets). Do not touch `Plugins/`.
- Do not rename/delete existing public classes, UFUNCTIONs, or UPROPERTYs (Blueprints reference them → "Derives from invalid class" crash, see `Docs/after/BUGS.md` BUG-01). Adding is fine; changing a signature that a Blueprint may use is not — add an overload/new function instead.
- No dependency on Blueprint assets. Everything must work from C++ defaults; where an asset reference is optional (montage, VFX, sound) use soft references and **degrade gracefully** (log once with the module log category + continue).
- Copy the existing style: tabs, header comment `// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.`, `#pragma once`, Doxygen-ish `/** */` on public API, categories `"AAA|Combat"` etc. Comments explain *why*, not *what*.

## UE correctness checklist (verify each in your self-review)
- `#include "X.generated.h"` is the **last** include in a header. Every type used in a `UPROPERTY/UFUNCTION` signature is included or forward-declared.
- Correct module `*_API` export macro on every class/struct/free function used across modules. Cross-module use ⇒ the consumer's `*.Build.cs` lists the provider module in `Public/PrivateDependencyModuleNames` (add it if missing, and say so in the report).
- Every `UCLASS` constructor/`BeginPlay`/`TickComponent`/`OnRegister` override calls `Super::`. `PrimaryComponentTick.bCanEverTick` only if Tick is really used.
- Replicated properties: `UPROPERTY(Replicated…)` + `GetLifetimeReplicatedProps` with `DOREPLIFETIME`/`DOREPLIFETIME_CONDITION`; `#include "Net/UnrealNetwork.h"`; `SetIsReplicatedByDefault(true)` for components. RPCs: `Server`/`Client`/`NetMulticast` + `Reliable/Unreliable` + `_Implementation` (+ `_Validate` only if `WithValidation`). Gameplay results are decided on authority (`HasAuthority()`); presentation (camera, HUD, hit-stop, shake) is local.
- Delegates: dynamic multicast need `UFUNCTION()` handlers bound with `AddDynamic` (and the handler must be a UFUNCTION). Lambdas capture `TWeakObjectPtr<>`, never raw `this` for timers/HTTP/async.
- Pointers: UObject members are `UPROPERTY()`-tracked (`TObjectPtr<>`/`TWeakObjectPtr<>`); check `IsValid()` before use; `GetWorld()` can be null in editor/CDO paths.
- Timers via `GetWorld()->GetTimerManager()`; clear them in `EndPlay`/`BeginDestroy` paths.
- GAS: use the project's existing `UAAAAbilitySystemComponent`, `UAAAAttributeSet`, `AAAGameplayTags` (native tags). New tags are added where the existing ones are declared (`AAACore/.../AAAGameplayTags.*`), same macro pattern. Use `ATTRIBUTE_ACCESSORS` macros as the existing attribute set does.
- Logging: define/declare a per-module log category if none exists; `UE_LOG(Cat, Verbosity, TEXT("... %s"), *FString)`.
- FString/FName/FText conversions explicit; `TArray` ranged-for doesn't mutate the array; no `auto` copies of large structs in hot paths.
- Every new `.cpp` includes its own header first. No unused includes of editor-only modules in runtime code (guard with `#if WITH_EDITOR`).

## Report format (`Docs/reports/<PROMPT_ID>_report.md`)
`## Plan` · `## Files` · `## Design decisions / deviations` · `## Could not verify` · `## Self-review results` (tick each checklist item that applied) · `## For the reviewer` (the 3 riskiest spots, file:line).

## Lessons from the first real compile (Claude, 2026-10-07) — these exact mistakes already happened; do not repeat
1. **Replicated loose tags in UE 5.8**: there is no `AddReplicatedLooseGameplayTag`. Use `ASC->AddLooseGameplayTag(Tag, 1, EGameplayTagReplicationState::TagOnly)` / `RemoveLooseGameplayTag(Tag, 1, EGameplayTagReplicationState::TagOnly)` (authority only).
2. **Module cycles break the build instantly.** Current graph facts: `AAACharacter → AAAAbility`, `AAAAnimation → AAACharacter`, `AAAAbility → AAACombat`, `AAAGame → AAAAbility/AAACharacter`. Therefore `AAAAbility` and `AAACombat` **cannot** depend on `AAAAnimation`, `AAACharacter` or `AAAGame`. Cross-module needs go through `AAACore` (interfaces, tags, settings) or by matching class names/UFUNCTION names at runtime. Check every `*.Build.cs` before adding an `#include` from another module.
3. **Every function you declare must be defined.** A declared-but-undefined `OnRep_*` / method compiles but fails at link time. Re-grep each new declaration for a definition.
4. **Header-only (inline) helper classes must NOT carry a `*_API` macro** (e.g. `FAAADebugLogger`), or consumers get unresolved dllimport symbols.
5. `FGameplayTag` in a `TMap`/member, `FOnAttributeChangeData`, `FAnimNotifyEvent` etc. need their real headers (`GameplayTagContainer.h`, `GameplayEffectTypes.h`, …) in the **header** that declares them; forward declarations are not enough.
6. A local variable named like a class member (`Damage`, `Health`) triggers warning-as-error C4458: use `Final…`/`Local…` names.
7. `LogTemp`-style or `GetActorInfo()` pitfalls: `GameplayAbility::CurrentActorInfo` is a pointer; `GetActorInfo()` returns a reference.
8. Your self-review checklist boxes must be ticked honestly — an unticked box in your report means you skipped that check.
9. A UBT compile is run by the reviewer after you finish; every error costs a round trip, so mentally compile each file (includes, API names against engine source, const-correctness, delegate signatures) before declaring done.
