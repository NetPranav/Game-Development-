// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAAItemTypes.h"
#include "AAAItemDefinition.generated.h"

class UTexture2D;
class UStaticMesh;
class USkeletalMesh;

/**
 * UAAAItemDefinition
 *
 * The base Data Asset representing an item archetype.
 * Contains all static, designer-authored properties of an item.
 * No runtime state should ever live in this class.
 *
 * Specialized item types (Weapons, Armor, etc.) derive from this class
 * to add category-specific properties.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAAAItemDefinition();

	//~ Begin UPrimaryDataAsset Interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End UPrimaryDataAsset Interface

	/* -------------------------------------------------------------------
	 * Identity
	 * ------------------------------------------------------------------- */

	/** Display name shown in UI and tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Identity")
	FText ItemName;

	/** Lore or gameplay description. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Identity", meta = (MultiLine = true))
	FText Description;

	/** Unique string identifier for serialization and lookup. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Identity")
	FName ItemID;

	/* -------------------------------------------------------------------
	 * Visual
	 * ------------------------------------------------------------------- */

	/** Icon displayed in inventory grids and tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Thumbnail for loot popups and quick previews. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Visual")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	/** Static mesh for world drops and environmental display. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Visual")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** Skeletal mesh for equipped items (weapons, armor). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Visual")
	TSoftObjectPtr<USkeletalMesh> EquippedMesh;

	/* -------------------------------------------------------------------
	 * Classification
	 * ------------------------------------------------------------------- */

	/** Broad item category. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Classification")
	EAAAItemCategory Category = EAAAItemCategory::Miscellaneous;

	/** Rarity tier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Classification")
	EAAAItemRarity Rarity = EAAAItemRarity::Common;

	/** Gameplay tag container for flexible filtering and queries. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Classification")
	FGameplayTagContainer ItemTags;

	/* -------------------------------------------------------------------
	 * Economics
	 * ------------------------------------------------------------------- */

	/** Weight per unit. Used for inventory weight limits. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Economics", meta = (ClampMin = "0.0"))
	float Weight = 0.0f;

	/** Maximum number of this item in a single stack. 1 = non-stackable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Economics", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	/** Base price when selling to a vendor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Economics", meta = (ClampMin = "0"))
	int32 SellValue = 0;

	/** Base price when buying from a vendor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Economics", meta = (ClampMin = "0"))
	int32 BuyValue = 0;

	/* -------------------------------------------------------------------
	 * Durability
	 * ------------------------------------------------------------------- */

	/** Whether this item has durability (weapons/armor). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Durability")
	bool bHasDurability = false;

	/** Maximum durability value. Only relevant if bHasDurability is true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Durability", meta = (EditCondition = "bHasDurability", ClampMin = "1.0"))
	float MaxDurability = 100.0f;

	/* -------------------------------------------------------------------
	 * Equipment
	 * ------------------------------------------------------------------- */

	/** If this item is equippable, which slot does it go into? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Equipment")
	EAAAEquipmentSlot EquipmentSlot = EAAAEquipmentSlot::None;
};
