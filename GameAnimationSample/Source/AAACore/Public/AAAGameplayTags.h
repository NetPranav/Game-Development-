#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Singleton struct containing native Gameplay Tags.
 * Native tags are safer and more performant than Blueprint-defined tags.
 */
struct AAACORE_API FAAAGameplayTags
{
public:
	static const FAAAGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	// State Tags
	FGameplayTag State_Alive;
	FGameplayTag State_Dead;
	FGameplayTag State_Stunned;
	FGameplayTag State_Knocked;
	FGameplayTag State_Interacting;
	FGameplayTag State_Combat;
	FGameplayTag State_Combat_Drawn;
	FGameplayTag State_Combat_Holstered;
	FGameplayTag State_Combat_Unarmed;
	FGameplayTag State_Combat_Invulnerable;
	FGameplayTag State_Combat_Blocking;
	FGameplayTag State_Combat_Parrying;
	FGameplayTag State_Combat_Staggered;
	FGameplayTag State_Combat_Attacking;
	FGameplayTag State_Combat_Dead;
	FGameplayTag State_Climbing;
	FGameplayTag State_Sprinting;
	FGameplayTag State_Crouching;

	// Ability Tags
	FGameplayTag Ability_None;

	// Damage Tags
	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Ice;
	FGameplayTag Damage_Poison;
	FGameplayTag Damage_Fall;

	// Status Tags
	FGameplayTag Status_Invincible;

	// Data Tags (used by SetByCallerMagnitude for zero-allocation GE)
	FGameplayTag Data_Damage;
	FGameplayTag Data_Healing;
	FGameplayTag Data_PoiseDamage;
	FGameplayTag Data_StaminaCost;

	FGameplayTag GameplayEvent_Combat_Hit;
	FGameplayTag GameplayEvent_Combat_Blocked;
	FGameplayTag GameplayEvent_Combat_Parried;
	FGameplayTag GameplayEvent_Combat_Staggered;
	FGameplayTag GameplayEvent_Combat_Death;
	FGameplayTag GameplayEvent_Combat_ComboInput;
	FGameplayTag GameplayEvent_Combat_ComboWindowOpen;
	FGameplayTag GameplayEvent_Combat_ComboWindowClose;

	// Character Tags
	FGameplayTag Character_Player;
	FGameplayTag Character_NPC;
	FGameplayTag Character_Boss;
	FGameplayTag State_Boss_Segmented;
	FGameplayTag State_Boss_Perilous;

	// Movement Tags
	FGameplayTag Movement_Walking;
	FGameplayTag Movement_Running;
	FGameplayTag Movement_Sprinting;
	FGameplayTag Movement_Jumping;
	FGameplayTag Movement_Falling;

	// -------------------------------------------------------------------
	// Item Tags (Phase 7)
	// -------------------------------------------------------------------
	FGameplayTag Item_Weapon;
	FGameplayTag Item_Armor;
	FGameplayTag Item_Consumable;
	FGameplayTag Item_Quest;
	FGameplayTag Item_Crafting;
	FGameplayTag Item_Dragon;
	FGameplayTag Item_Currency;
	FGameplayTag Item_Food;
	FGameplayTag Item_Potion;
	FGameplayTag Item_Rare;
	FGameplayTag Item_Legendary;
	FGameplayTag Item_TwoHanded;
	FGameplayTag Item_OneHanded;

	// -------------------------------------------------------------------
	// Equipment Tags (Phase 7)
	// -------------------------------------------------------------------
	FGameplayTag Equipment_MainHand;
	FGameplayTag Equipment_OffHand;
	FGameplayTag Equipment_Head;
	FGameplayTag Equipment_Chest;
	FGameplayTag Equipment_Legs;
	FGameplayTag Equipment_Feet;
	FGameplayTag Equipment_Gloves;
	FGameplayTag Equipment_Ring;
	FGameplayTag Equipment_Necklace;
	FGameplayTag Equipment_Dragon;

	// -------------------------------------------------------------------
	// Input Tags (Tag-Based Ability Activation — Lyra Pattern)
	// -------------------------------------------------------------------
	FGameplayTag InputTag_Attack;
	FGameplayTag InputTag_AttackHeavy;
	FGameplayTag InputTag_SecondaryAttack;
	FGameplayTag InputTag_Ability1;
	FGameplayTag InputTag_Ability2;
	FGameplayTag InputTag_Ability3;
	FGameplayTag InputTag_Ability4;
	FGameplayTag InputTag_Interact;
	FGameplayTag InputTag_Dodge;
	FGameplayTag InputTag_Sprint;
	FGameplayTag InputTag_Jump;
	FGameplayTag InputTag_DrawSheath;

	// -------------------------------------------------------------------
	// GameplayCue Tags (VFX/Audio — zero cosmetic replication)
	// -------------------------------------------------------------------
	FGameplayTag GameplayCue_Combat_Hit_Melee;
	FGameplayTag GameplayCue_Combat_Hit_Sword;
	FGameplayTag GameplayCue_Combat_Hit_Impact;
	FGameplayTag GameplayCue_Combat_Hit_Blocked;
	FGameplayTag GameplayCue_Combat_Hit_Parried;
	FGameplayTag GameplayCue_Character_Death;
	FGameplayTag GameplayCue_Character_Heal;

private:
	static FAAAGameplayTags GameplayTags;
};
