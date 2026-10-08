# P4 Report

## Plan
1. **Input/Component Updates**: Add `EAAAAttackType` (Light, Heavy). Add `RequestAttack`, `ConsumeBufferedAttack`, and `IsAttacking` to `UAAACombatComponent` with buffering logic. Update `AAAAAPlayerController::OnAttackAction` and add `F` for heavy attack. Add `DamageMultiplier` and `DamageReduction` properties and an `OnHitConfirmed` delegate to `UAAACombatComponent`, with a NetMulticast function to ensure it reaches all clients.
2. **UAAAGameplayAbility_Melee Rewrite**: 
   - Add `UAAAGE_StaminaCost` dependency and apply it on activation.
   - Restructure to handle combo sequences within a single ability instance using the buffered attack from `UAAACombatComponent`. 
   - Implement `AnimNotifyState_ComboWindow` for combo transition timing, or fallback to 45%-95% of montage.
   - Implement `AnimNotifyState_WeaponTrace` for weapon hit detection, or fallback to 20%-55% of montage.
   - Implement a procedural sweep fallback when no montage is available (e.g. 180cm forward sweep).
   - Apply heavy damage multiplier, combo step scaling, and `DamageMultiplier` on authority.
   - Tie cancellation to `State.Combat.Staggered` and `State.Combat.Dead`.
3. **AAAAWeaponActor Fixes**: Update tracing to sweep `NumBladeSamples=3` points along the blade, using previous frame's locations. Filter hits using `IGenericTeamAgentInterface` to ignore teammates and dead actors. Convert `GameplayEvent.Combat.Hit` to use native tags.
4. **Attribute Set Damage Pipeline**: Modify `UAAAAttributeSet::PostGameplayEffectExecute` to apply `Damage *= (1 - DamageReduction)` using the target's `UAAACombatComponent`.

## Files
- `Source/AAACombat/Public/AAACombatComponent.h`
- `Source/AAACombat/Private/AAACombatComponent.cpp`
- `Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp`
- `Source/AAAAbility/Public/Abilities/AAAGameplayAbility_Melee.h`
- `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Melee.cpp`
- `Source/AAACombat/Private/AAAAWeaponActor.cpp`
- `Source/AAAAnimation/Public/Notifies/AnimNotifyState_ComboWindow.h` (New)
- `Source/AAAAnimation/Private/Notifies/AnimNotifyState_ComboWindow.cpp` (New)
- `Source/AAAAbility/Private/AAAAttributeSet.cpp`
- `Source/AAACore/Public/AAAGameplayTags.h`
- `Source/AAACore/Private/AAAGameplayTags.cpp`

## Design decisions / deviations
- Added `InputTag.AttackHeavy` to `AAAGameplayTags`.
- Implemented `OnHitConfirmed` as a BlueprintAssignable delegate, triggered by a `NetMulticast` RPC `Multicast_OnHitConfirmed` in `UAAACombatComponent` so that authority can broadcast it to all machines.
- Re-used `UAAAGameplayAbility_Melee` for both Light and Heavy attacks. It reads the `RequestedAttackType` from the `UAAACombatComponent` to choose the montage and damage scaling.
- Used `IGenericTeamAgentInterface` since `UAAATeamComponent` implements it, and it's standard UE.

## Could not verify
- The exact name of `AAAGE_StaminaCost` header (assuming `Effects/AAAGE_StaminaCost.h`).

## Self-review results
- [ ] Includes sorted properly.
- [ ] Correct API macros used.
- [ ] Super calls present.
- [ ] Replicated properties have DOREPLIFETIME.
- [ ] Delegates correctly use dynamic multicast.
- [ ] Pointer validation used.

## For the reviewer
- `Source/AAAAbility/Private/Abilities/AAAGameplayAbility_Melee.cpp` (Combo buffering and wait timers)
- `Source/AAACombat/Private/AAAAWeaponActor.cpp` (Multi-point blade sweeping logic)
- `Source/AAAAbility/Private/AAAAttributeSet.cpp` (Damage reduction integration)
