#include "AAAGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Engine/EngineTypes.h"

FAAAGameplayTags FAAAGameplayTags::GameplayTags;

void FAAAGameplayTags::InitializeNativeGameplayTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// State Tags
	GameplayTags.State_Alive = Manager.AddNativeGameplayTag(FName("State.Alive"), FString("Character is currently alive."));
	GameplayTags.State_Dead = Manager.AddNativeGameplayTag(FName("State.Dead"), FString("Character has died."));
	GameplayTags.State_Stunned = Manager.AddNativeGameplayTag(FName("State.Stunned"), FString("Character is stunned and cannot act."));
	GameplayTags.State_Knocked = Manager.AddNativeGameplayTag(FName("State.Knocked"), FString("Character is knocked down."));
	GameplayTags.State_Interacting = Manager.AddNativeGameplayTag(FName("State.Interacting"), FString("Character is interacting with an object."));
	GameplayTags.State_Combat = Manager.AddNativeGameplayTag(FName("State.Combat"), FString("Character is currently in a combat state."));
	GameplayTags.State_Combat_Drawn = Manager.AddNativeGameplayTag(FName("State.Combat.Drawn"), FString("Weapon is drawn in hand."));
	GameplayTags.State_Combat_Holstered = Manager.AddNativeGameplayTag(FName("State.Combat.Holstered"), FString("Weapon is holstered on body."));
	GameplayTags.State_Combat_Unarmed = Manager.AddNativeGameplayTag(FName("State.Combat.Unarmed"), FString("Character is unarmed / barehanded."));
	GameplayTags.State_Combat_Invulnerable = Manager.AddNativeGameplayTag(FName("State.Combat.Invulnerable"), FString("Character is currently in i-frames (dodge/roll)."));
	GameplayTags.State_Combat_Blocking = Manager.AddNativeGameplayTag(FName("State.Combat.Blocking"), FString("Character is blocking."));
	GameplayTags.State_Combat_Parrying = Manager.AddNativeGameplayTag(FName("State.Combat.Parrying"), FString("Character is parrying."));
	GameplayTags.State_Combat_Staggered = Manager.AddNativeGameplayTag(FName("State.Combat.Staggered"), FString("Character is staggered."));
	GameplayTags.State_Combat_Attacking = Manager.AddNativeGameplayTag(FName("State.Combat.Attacking"), FString("Character is attacking."));
	GameplayTags.State_Combat_Dead = Manager.AddNativeGameplayTag(FName("State.Combat.Dead"), FString("Character is dead (combat)."));
	GameplayTags.State_Climbing = Manager.AddNativeGameplayTag(FName("State.Climbing"), FString("Character is climbing."));
	GameplayTags.State_Sprinting = Manager.AddNativeGameplayTag(FName("State.Sprinting"), FString("Character is sprinting."));
	GameplayTags.State_Crouching = Manager.AddNativeGameplayTag(FName("State.Crouching"), FString("Character is crouching."));

	// Ability Tags
	GameplayTags.Ability_None = Manager.AddNativeGameplayTag(FName("Ability.None"), FString("No specific ability."));

	// Damage Tags
	GameplayTags.Damage_Physical = Manager.AddNativeGameplayTag(FName("Damage.Physical"), FString("Physical damage type."));
	GameplayTags.Damage_Fire = Manager.AddNativeGameplayTag(FName("Damage.Fire"), FString("Fire damage type."));
	GameplayTags.Damage_Ice = Manager.AddNativeGameplayTag(FName("Damage.Ice"), FString("Ice damage type."));
	GameplayTags.Damage_Poison = Manager.AddNativeGameplayTag(FName("Damage.Poison"), FString("Poison damage type."));
	GameplayTags.Damage_Fall = Manager.AddNativeGameplayTag(FName("Damage.Fall"), FString("Fall damage type."));

	// Status Tags
	GameplayTags.Status_Invincible = Manager.AddNativeGameplayTag(FName("Status.Invincible"), FString("Character takes no damage."));

	// Data Tags (for SetByCallerMagnitude — zero-allocation damage/healing)
	GameplayTags.Data_Damage = Manager.AddNativeGameplayTag(FName("Data.Damage"), FString("SetByCaller magnitude tag for damage values."));
	GameplayTags.Data_Healing = Manager.AddNativeGameplayTag(FName("Data.Healing"), FString("SetByCaller magnitude tag for healing values."));
	GameplayTags.Data_PoiseDamage = Manager.AddNativeGameplayTag(FName("Data.PoiseDamage"), FString("SetByCaller magnitude tag for poise damage."));
	GameplayTags.Data_StaminaCost = Manager.AddNativeGameplayTag(FName("Data.StaminaCost"), FString("SetByCaller magnitude tag for stamina cost."));

	// Gameplay Event Tags
	GameplayTags.GameplayEvent_Combat_Hit = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.Hit"), FString("Fired when a weapon trace hits an enemy."));
	GameplayTags.GameplayEvent_Combat_Blocked = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.Blocked"), FString("Fired when an attack is blocked."));
	GameplayTags.GameplayEvent_Combat_Parried = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.Parried"), FString("Fired when an attack is parried."));
	GameplayTags.GameplayEvent_Combat_Staggered = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.Staggered"), FString("Fired when a character is staggered."));
	GameplayTags.GameplayEvent_Combat_Death = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.Death"), FString("Fired when a character dies."));
	GameplayTags.GameplayEvent_Combat_ComboInput = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.ComboInput"), FString("Combo input received."));
	GameplayTags.GameplayEvent_Combat_ComboWindowOpen = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.ComboWindowOpen"), FString("Combo window open."));
	GameplayTags.GameplayEvent_Combat_ComboWindowClose = Manager.AddNativeGameplayTag(FName("GameplayEvent.Combat.ComboWindowClose"), FString("Combo window closed."));

	// Character Tags
	GameplayTags.Character_Player = Manager.AddNativeGameplayTag(FName("Character.Player"), FString("Tag for Player Characters."));
	GameplayTags.Character_NPC = Manager.AddNativeGameplayTag(FName("Character.NPC"), FString("Tag for Non-Playable Characters."));
	GameplayTags.Character_Boss = Manager.AddNativeGameplayTag(FName("Character.Boss"), FString("Tag for Boss Enemies."));
	GameplayTags.State_Boss_Segmented = Manager.AddNativeGameplayTag(FName("State.Boss.Segmented"), FString("Boss has health segments."));
	GameplayTags.State_Boss_Perilous = Manager.AddNativeGameplayTag(FName("State.Boss.Perilous"), FString("Boss is doing a perilous attack."));

	// Movement Tags
	GameplayTags.Movement_Walking = Manager.AddNativeGameplayTag(FName("Movement.Walking"), FString("Character is walking."));
	GameplayTags.Movement_Running = Manager.AddNativeGameplayTag(FName("Movement.Running"), FString("Character is running."));
	GameplayTags.Movement_Sprinting = Manager.AddNativeGameplayTag(FName("Movement.Sprinting"), FString("Movement mode: Sprinting."));
	GameplayTags.Movement_Jumping = Manager.AddNativeGameplayTag(FName("Movement.Jumping"), FString("Character is jumping."));
	GameplayTags.Movement_Falling = Manager.AddNativeGameplayTag(FName("Movement.Falling"), FString("Character is falling."));

	// -------------------------------------------------------------------
	// Item Tags (Phase 7)
	// -------------------------------------------------------------------
	GameplayTags.Item_Weapon = Manager.AddNativeGameplayTag(FName("Item.Weapon"), FString("Item is a weapon."));
	GameplayTags.Item_Armor = Manager.AddNativeGameplayTag(FName("Item.Armor"), FString("Item is armor."));
	GameplayTags.Item_Consumable = Manager.AddNativeGameplayTag(FName("Item.Consumable"), FString("Item is consumable."));
	GameplayTags.Item_Quest = Manager.AddNativeGameplayTag(FName("Item.Quest"), FString("Item is a quest item."));
	GameplayTags.Item_Crafting = Manager.AddNativeGameplayTag(FName("Item.Crafting"), FString("Item is a crafting material."));
	GameplayTags.Item_Dragon = Manager.AddNativeGameplayTag(FName("Item.Dragon"), FString("Item is dragon equipment."));
	GameplayTags.Item_Currency = Manager.AddNativeGameplayTag(FName("Item.Currency"), FString("Item is currency."));
	GameplayTags.Item_Food = Manager.AddNativeGameplayTag(FName("Item.Food"), FString("Item is food."));
	GameplayTags.Item_Potion = Manager.AddNativeGameplayTag(FName("Item.Potion"), FString("Item is a potion."));
	GameplayTags.Item_Rare = Manager.AddNativeGameplayTag(FName("Item.Rare"), FString("Item has rare quality."));
	GameplayTags.Item_Legendary = Manager.AddNativeGameplayTag(FName("Item.Legendary"), FString("Item has legendary quality."));
	GameplayTags.Item_TwoHanded = Manager.AddNativeGameplayTag(FName("Item.TwoHanded"), FString("Item requires two hands."));
	GameplayTags.Item_OneHanded = Manager.AddNativeGameplayTag(FName("Item.OneHanded"), FString("Item is one-handed."));

	// -------------------------------------------------------------------
	// Equipment Tags (Phase 7)
	// -------------------------------------------------------------------
	GameplayTags.Equipment_MainHand = Manager.AddNativeGameplayTag(FName("Equipment.MainHand"), FString("Equipment slot: Main Hand."));
	GameplayTags.Equipment_OffHand = Manager.AddNativeGameplayTag(FName("Equipment.OffHand"), FString("Equipment slot: Off Hand."));
	GameplayTags.Equipment_Head = Manager.AddNativeGameplayTag(FName("Equipment.Head"), FString("Equipment slot: Head."));
	GameplayTags.Equipment_Chest = Manager.AddNativeGameplayTag(FName("Equipment.Chest"), FString("Equipment slot: Chest."));
	GameplayTags.Equipment_Legs = Manager.AddNativeGameplayTag(FName("Equipment.Legs"), FString("Equipment slot: Legs."));
	GameplayTags.Equipment_Feet = Manager.AddNativeGameplayTag(FName("Equipment.Feet"), FString("Equipment slot: Feet."));
	GameplayTags.Equipment_Gloves = Manager.AddNativeGameplayTag(FName("Equipment.Gloves"), FString("Equipment slot: Gloves."));
	GameplayTags.Equipment_Ring = Manager.AddNativeGameplayTag(FName("Equipment.Ring"), FString("Equipment slot: Ring."));
	GameplayTags.Equipment_Necklace = Manager.AddNativeGameplayTag(FName("Equipment.Necklace"), FString("Equipment slot: Necklace."));
	GameplayTags.Equipment_Dragon = Manager.AddNativeGameplayTag(FName("Equipment.Dragon"), FString("Equipment slot: Dragon."));

	// -------------------------------------------------------------------
	// Input Tags (Tag-Based Ability Activation — Lyra Pattern)
	// -------------------------------------------------------------------
	GameplayTags.InputTag_Attack = Manager.AddNativeGameplayTag(FName("InputTag.Attack"), FString("Input: Primary attack."));
	GameplayTags.InputTag_AttackHeavy = Manager.AddNativeGameplayTag(FName("InputTag.AttackHeavy"), FString("Input: Heavy attack."));
	GameplayTags.InputTag_SecondaryAttack = Manager.AddNativeGameplayTag(FName("InputTag.SecondaryAttack"), FString("Input: Secondary/heavy attack."));
	GameplayTags.InputTag_Ability1 = Manager.AddNativeGameplayTag(FName("InputTag.Ability1"), FString("Input: Ability slot 1."));
	GameplayTags.InputTag_Ability2 = Manager.AddNativeGameplayTag(FName("InputTag.Ability2"), FString("Input: Ability slot 2."));
	GameplayTags.InputTag_Ability3 = Manager.AddNativeGameplayTag(FName("InputTag.Ability3"), FString("Input: Ability slot 3."));
	GameplayTags.InputTag_Ability4 = Manager.AddNativeGameplayTag(FName("InputTag.Ability4"), FString("Input: Ability slot 4."));
	GameplayTags.InputTag_Interact = Manager.AddNativeGameplayTag(FName("InputTag.Interact"), FString("Input: Interact with world object."));
	GameplayTags.InputTag_Dodge = Manager.AddNativeGameplayTag(FName("InputTag.Dodge"), FString("Input: Dodge/roll."));
	GameplayTags.InputTag_Sprint = Manager.AddNativeGameplayTag(FName("InputTag.Sprint"), FString("Input: Sprint (hold)."));
	GameplayTags.InputTag_Jump = Manager.AddNativeGameplayTag(FName("InputTag.Jump"), FString("Input: Jump."));
	GameplayTags.InputTag_DrawSheath = Manager.AddNativeGameplayTag(FName("InputTag.DrawSheath"), FString("Input: Draw/Sheath weapon."));

	// -------------------------------------------------------------------
	// GameplayCue Tags (VFX/Audio — zero cosmetic replication, pooled)
	// -------------------------------------------------------------------
	GameplayTags.GameplayCue_Combat_Hit_Melee = Manager.AddNativeGameplayTag(FName("GameplayCue.Combat.Hit.Melee"), FString("VFX/SFX: Generic melee hit."));
	GameplayTags.GameplayCue_Combat_Hit_Sword = Manager.AddNativeGameplayTag(FName("GameplayCue.Combat.Hit.Sword"), FString("VFX/SFX: Sword weapon hit."));
	GameplayTags.GameplayCue_Combat_Hit_Impact = Manager.AddNativeGameplayTag(FName("GameplayCue.Combat.Hit.Impact"), FString("VFX/SFX: Heavy impact hit."));
	GameplayTags.GameplayCue_Combat_Hit_Blocked = Manager.AddNativeGameplayTag(FName("GameplayCue.Combat.Hit.Blocked"), FString("VFX/SFX: Blocked hit."));
	GameplayTags.GameplayCue_Combat_Hit_Parried = Manager.AddNativeGameplayTag(FName("GameplayCue.Combat.Hit.Parried"), FString("VFX/SFX: Parried hit."));
	GameplayTags.GameplayCue_Character_Death = Manager.AddNativeGameplayTag(FName("GameplayCue.Character.Death"), FString("VFX/SFX: Character death effect."));
	GameplayTags.GameplayCue_Character_Heal = Manager.AddNativeGameplayTag(FName("GameplayCue.Character.Heal"), FString("VFX/SFX: Healing effect."));
}
