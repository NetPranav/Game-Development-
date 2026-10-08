## Plan
1. Create `AAACombatEvents.h` and `AAACombatEvents.cpp` containing `FAAACombatEvents` and static accessors to native multicast delegates for `OnHitConfirmed`, `OnBlocked`, `OnParried`, `OnStaggered`, and `OnDeath`.
2. Add `FAAAHitReactionSet` (USTRUCT) to `AAACombatComponent.h` with `Front`, `Back`, `Left`, `Right`, `Stagger`, and `Death` AnimMontages.
3. Add `HitReactions` property to `UAAACombatComponent`.
4. Register callbacks for `GameplayEvent.Combat.Blocked`, `Staggered`, and `Death` in `UAAACombatComponent::InitializeRegen` (alongside `Parried`) to broadcast the native delegates and handle Staggered/Death hit reactions. Add `FDelegateHandle` for each to unbind in `EndPlay`.
5. In `UAAACombatComponent::Multicast_OnHitConfirmed_Implementation`, broadcast the native `OnHitConfirmed` delegate, and if the victim is this component's owner, `Damage >= HitReactMinDamageFraction` of `MaxHealth`, and not blocking, play the directional hit reaction.
6. In Staggered event, play `HitReactions.Stagger` if set.
7. In Death event, play `HitReactions.Death` if set. Otherwise, check if owner is an `APawn` without a Mover component, and if so, simulate ragdoll physics.
8. Update `UAAACombatSettings` with `KnockbackLight`, `KnockbackHeavy`, `KnockbackStagger`, and `HitReactMinDamageFraction`.

## Files
- CREATE `Source/AAACombat/Public/AAACombatEvents.h`
- CREATE `Source/AAACombat/Private/AAACombatEvents.cpp`
- EDIT `Source/AAACombat/Public/AAACombatComponent.h`
- EDIT `Source/AAACombat/Private/AAACombatComponent.cpp`
- EDIT `Source/AAACombat/Public/AAACombatSettings.h`

## Design decisions / deviations
- Added `FAAAHitReactionSet` into `AAACombatComponent.h` as instructed.
- Checking for the Mover component is done by iterating over components and checking if the class name contains "MoverComponent" to avoid introducing a Mover dependency into `AAACombat`.

## Could not verify
-

## Self-review results
- [ ] `#include "X.generated.h"` is the last include in a header.
- [ ] Correct module `*_API` export macro on every class/struct/free function.
- [ ] Every `UCLASS` constructor/BeginPlay... overrides call `Super::`.
- [ ] Replicated properties properly setup.
- [ ] Delegates use correct bindings.
- [ ] Pointers checked.
- [ ] Timers cleared.
- [ ] GAS utilized correctly.
- [ ] FString/FName conversions explicit.
- [ ] No unused includes.

## For the reviewer
- `Source/AAACombat/Private/AAACombatComponent.cpp` (Hit reaction and ragdoll logic without Mover dependency)
