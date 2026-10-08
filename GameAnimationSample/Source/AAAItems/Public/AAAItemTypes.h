// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemTypes.generated.h"

/* -------------------------------------------------------------------
 * Item Category
 * Determines the broad classification of an item.
 * ------------------------------------------------------------------- */
UENUM(BlueprintType)
enum class EAAAItemCategory : uint8
{
	None			UMETA(DisplayName = "None"),
	Weapon			UMETA(DisplayName = "Weapon"),
	Armor			UMETA(DisplayName = "Armor"),
	Consumable		UMETA(DisplayName = "Consumable"),
	QuestItem		UMETA(DisplayName = "Quest Item"),
	CraftingMaterial	UMETA(DisplayName = "Crafting Material"),
	KeyItem			UMETA(DisplayName = "Key Item"),
	DragonEquipment		UMETA(DisplayName = "Dragon Equipment"),
	Resource		UMETA(DisplayName = "Resource"),
	Currency		UMETA(DisplayName = "Currency"),
	Book			UMETA(DisplayName = "Book"),
	SkillTome		UMETA(DisplayName = "Skill Tome"),
	Food			UMETA(DisplayName = "Food"),
	Potion			UMETA(DisplayName = "Potion"),
	Miscellaneous		UMETA(DisplayName = "Miscellaneous"),
};

/* -------------------------------------------------------------------
 * Item Rarity
 * Controls loot tables, visual presentation, and value scaling.
 * ------------------------------------------------------------------- */
UENUM(BlueprintType)
enum class EAAAItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary"),
	Mythic		UMETA(DisplayName = "Mythic"),
	Ancient		UMETA(DisplayName = "Ancient"),
	Divine		UMETA(DisplayName = "Divine"),
	Developer	UMETA(DisplayName = "Developer"),
};

/* -------------------------------------------------------------------
 * Item Quality
 * Represents the crafting quality or condition of an individual item.
 * ------------------------------------------------------------------- */
UENUM(BlueprintType)
enum class EAAAItemQuality : uint8
{
	Poor		UMETA(DisplayName = "Poor"),
	Normal		UMETA(DisplayName = "Normal"),
	Fine		UMETA(DisplayName = "Fine"),
	Superior	UMETA(DisplayName = "Superior"),
	Masterwork	UMETA(DisplayName = "Masterwork"),
	Legendary	UMETA(DisplayName = "Legendary"),
};

/* -------------------------------------------------------------------
 * Armor Type
 * Determines movement penalties, animation overrides, and resistances.
 * Note: EAAAWeaponType lives in AAACombat module to avoid duplication.
 * ------------------------------------------------------------------- */
UENUM(BlueprintType)
enum class EAAAArmorType : uint8
{
	None	UMETA(DisplayName = "None"),
	Light	UMETA(DisplayName = "Light"),
	Medium	UMETA(DisplayName = "Medium"),
	Heavy	UMETA(DisplayName = "Heavy"),
	Mage	UMETA(DisplayName = "Mage"),
	Dragon	UMETA(DisplayName = "Dragon"),
};

/* -------------------------------------------------------------------
 * Equipment Slot
 * Strongly typed slot identifiers for the Equipment Component.
 * ------------------------------------------------------------------- */
UENUM(BlueprintType)
enum class EAAAEquipmentSlot : uint8
{
	None		UMETA(DisplayName = "None"),
	Head		UMETA(DisplayName = "Head"),
	Chest		UMETA(DisplayName = "Chest"),
	Legs		UMETA(DisplayName = "Legs"),
	Feet		UMETA(DisplayName = "Feet"),
	Gloves		UMETA(DisplayName = "Gloves"),
	RingLeft	UMETA(DisplayName = "Ring Left"),
	RingRight	UMETA(DisplayName = "Ring Right"),
	Necklace	UMETA(DisplayName = "Necklace"),
	MainWeapon	UMETA(DisplayName = "Main Weapon"),
	Offhand		UMETA(DisplayName = "Offhand"),
	Bow		UMETA(DisplayName = "Bow"),
	Shield		UMETA(DisplayName = "Shield"),
	DragonSaddle	UMETA(DisplayName = "Dragon Saddle"),
	DragonArmor	UMETA(DisplayName = "Dragon Armor"),
	DragonAccessory	UMETA(DisplayName = "Dragon Accessory"),
};
