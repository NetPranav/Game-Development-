# P6a — Combat event bus + hit reactions (module AAACombat only)
Read first: `Docs/prompts/00_RULES.md`, `Docs/reports/MODULE_DEPS.md`, `Docs/reports/FILE_INDEX.md`. **Never call run_command or any shell tool — a denied shell call ends your session. You do not need to list directories: every path you need is named below; directories already exist.** Spec source: sections 1 and 2 of `Docs/prompts/P6_reactions_feedback_lockon.md` plus its "Reviewer decisions" section (they override).

## Files you will touch (exact paths, relative to the workspace root `GameAnimationSample/`)
- CREATE `Source/AAACombat/Public/AAACombatEvents.h`
- CREATE `Source/AAACombat/Private/AAACombatEvents.cpp`
- EDIT `Source/AAACombat/Public/AAACombatComponent.h`
- EDIT `Source/AAACombat/Private/AAACombatComponent.cpp`
- EDIT `Source/AAACombat/Public/AAACombatSettings.h` (and `Private/AAACombatSettings.cpp` only if needed)
Read these existing files fully before editing: `Source/AAACombat/Public/AAACombatComponent.h`, `Source/AAACombat/Private/AAACombatComponent.cpp`, `Source/AAACombat/Public/AAACombatSettings.h`, `Source/AAACore/Public/AAAGameplayTags.h`, and the reports `Docs/reports/P4_report.md`, `Docs/reports/P5_report.md`.

## Do exactly this
1. `FAAACombatEvents` (export `AAACOMBAT_API`): static accessor functions returning references to function-local static native multicast delegates: `OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult&, bool bHeavy, float Damage)`, `OnBlocked(AActor* Attacker, AActor* Victim)`, `OnParried(AActor* Attacker, AActor* Victim)`, `OnStaggered(AActor* Victim)`, `OnDeath(AActor* Victim)`.
2. `UAAACombatComponent` broadcasts them on **every machine**: where P4's `OnHitConfirmed` (dynamic delegate / multicast RPC) fires, add the native broadcast; register ASC generic-event callbacks (same place the Parried callback is registered) for `GameplayEvent.Combat.Blocked/Staggered/Death` to broadcast `OnBlocked/OnStaggered/OnDeath`. Unbind in `EndPlay`.
3. Add `struct FAAAHitReactionSet` (USTRUCT BlueprintType; members `Front, Back, Left, Right, Stagger, Death`, each `TObjectPtr<UAnimMontage>`, EditAnywhere + BlueprintReadWrite) in `AAACombatComponent.h` (or a new header in `Source/AAACombat/Public`) and `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA|Combat|HitReactions") FAAAHitReactionSet HitReactions;` on the component.
4. Hit reactions (presentation, every machine): on `Staggered` play `HitReactions.Stagger` if set; on a damaging confirmed hit on **this component's owner** with `Damage >= HitReactMinDamageFraction` of MaxHealth (read MaxHealth through the ASC attribute-by-name helper already used for regen; no AttributeSet include) and not blocking: pick Front/Back/Left/Right from `(Victim location - Attacker location)` against the victim's forward/right vectors and play it; on `Death` play `HitReactions.Death` if set, else (only if the owner is NOT an `APawn` driven by a Mover component, i.e. a classic character) enable ragdoll: `SetSimulatePhysics(true)`, `SetCollisionProfileName("Ragdoll")`. Play via `USkeletalMeshComponent::GetAnimInstance()->Montage_Play`; null-check everything; never crash if there is no mesh/anim instance. Do not implement knockback here (the next prompt does it in another module).
5. Add to `UAAACombatSettings` (Config, EditAnywhere, additive only): `KnockbackLight=250`, `KnockbackHeavy=450`, `KnockbackStagger=600` (cm/s) and `HitReactMinDamageFraction=0.06`.

Finish: write `Docs/reports/P6a_report.md` (format in `00_RULES.md`, honest self-review ticks) and append a line `P6a done` to `Docs/PROGRESS.md`.
