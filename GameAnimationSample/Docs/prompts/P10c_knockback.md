# P10c — knockback fix
Read first: `Docs/prompts/00_RULES.md`, `Docs/SPEC_BOSS.md` (the whole spec), `Docs/reports/MODULE_DEPS.md`, `Docs/reports/FILE_INDEX.md`. **Do ONLY this prompt's deliverable, nothing else. Keep the work small and finish. Never call run_command or any shell tool. Do not delegate. Do not compile.** Verify engine APIs by viewing engine source under `A:/UNREAL/UE_5.8/Engine`.


## Why this prompt exists (measured facts, verify by reading the code)
1. **Nobody reacts to hits.** `FAAAHitReactionSet` (`AAACombat/Public/AAACombatComponent.h`: Front/Back/Left/Right/Stagger/Death montages) is filled on `BP_PlayerCharacter` and on `BP_RREnemy_Greystone` (`/Game/RR/Greystone/AM_GS_*`), but **no C++ ever plays them** except `ARREnemy::OnDeath` (Death). `UAAACombatComponent::Multicast_OnHitConfirmed`, `OnBlockedEvent`, `OnParriedEvent`, `OnStaggeredEvent`, `OnDeathEvent` do not play any montage.
2. **Block/Equip/Unequip montages only play for `ACharacter`** (`Cast<ACharacter>(GetOwner())->GetMesh()`), but the player is a Mover `APawn` (not an ACharacter). So the player never plays block, deflect, hit-react or stagger animations. For the player the pose is produced by the **master skeletal mesh** (the first `USkeletalMeshComponent` owned directly by the pawn, which is hidden in game; the visible robot is a child-actor display mesh that copies its pose through an AnimBP). Montages must therefore be played on the **master mesh's AnimInstance** (slot `DefaultSlot`).
3. **No knockback on the player / enemy** is visible: `UAAAKnockbackSubsystem` (`AAAAbility/Private/AAAKnockbackSubsystem.cpp`) reacts to `FAAACombatEvents::OnHitConfirmed`, but players don't move when hit. Find out why (Mover pawn path via `UAAAMoverUtils::ApplyVelocityBurst`, tags/state, direction, magnitude from `UAAACombatSettings`, victim dead/invulnerable checks, `CustomTimeDilation` hit-stop swallowing the impulse, characters needing `LaunchCharacter`) and fix it so a hit visibly pushes the victim 80-150 cm away (light) / 180-260 cm (heavy) over ~0.2 s, and a **blocked** hit pushes the defender back 40-70 cm with the slide feeling grounded. Both for ACharacter (enemy, use `LaunchCharacter` or `ApplyVelocityBurst`'s ACharacter path) and Mover pawns.
4. **Deflect (parry) has no feedback**: tapping block opens `State.Combat.Parrying` for `UAAACombatSettings::ParryWindowSeconds`; `UAAAAttributeSet::PostGameplayEffectExecute` swallows damage when parrying and sends `GameplayEvent_Combat_Parried` to the attacker, which then gets `Staggered`. There is no spark, no clang, no animation on the defender, and no poise damage to the attacker.
5. **The player still has a health bar** drawn in `RRGame/Private/RRHUD.cpp::DrawMatchHUD`. The user wants it replaced by a screen effect like Resident Evil 8 (see `Docs/SPEC_BOSS.md`, "Visual feedback").

## Deliverable C — knockback fix
Fix as described in "Why" item 3 and write in the report what the actual cause was. Knockback of the victim must not happen when the hit was deflected; blocked hits push the defender back (see item 3).


## Constraints specific to this prompt
- Keep `AAACombat` free of dependencies on `AAAAbility/AAACharacter/AAAGame/RRGame`. The poise damage on deflect is applied from `AAAAbility` (it already depends on AAACombat) — put cross-module logic there.
- Everything runs in single-player/listen-server authority; still keep RPC/multicast structure sane (presentation local, state on authority).
- Do not edit `.uasset` files. List in the report which montage/asset properties the reviewer must set in the editor (e.g. `DA_PlayerWeapon.Animations.DeflectMontage`).
- Report `Docs/reports/P10c_report.md` per `00_RULES.md`, then append `P10c done` to `Docs/PROGRESS.md`.
