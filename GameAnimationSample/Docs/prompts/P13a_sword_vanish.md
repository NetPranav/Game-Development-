# P13a - Sword vanishes when holstered (no sword on the back)

Small, exact task. Only edit the two files named below. Report to Docs/reports/P13a_report.md (what you changed, line ranges).

## 1. Source/AAACombat/Private/AAACombatComponent.cpp
- `UAAACombatComponent::AttachWeaponToHolster()` (around line 380): keep the attach to the holster socket (so the weapon
  still follows the body), but then HIDE the weapon: `CurrentWeapon->SetActorHiddenInGame(true);` and
  `CurrentWeapon->SetActorEnableCollision(false);`.
- `UAAACombatComponent::AttachWeaponToHand()` (just above it, ends with `SetActorRelativeTransform(... HandOffset)`):
  after attaching, SHOW the weapon again: `CurrentWeapon->SetActorHiddenInGame(false);` and
  `CurrentWeapon->SetActorEnableCollision(true);`.
- Do not change anything else in that file (other people are editing other functions in it).

## 2. Source/AAAGame/Private/GameFramework/AAAAPlayerController.cpp
- In `SetupInputComponent()` there is `InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AAAAAPlayerController::OnDrawSheathWeapon);`
  Add right below it three more bindings to the same handler: `EKeys::One`, `EKeys::MouseScrollUp`, `EKeys::MouseScrollDown`.
- In `OnDrawSheathWeapon()` remove the `GEngine->AddOnScreenDebugMessage(...)` "Weapon Stance" debug print (keep the toggle call).

Nothing else. No new files. Do not touch Build.cs files.
