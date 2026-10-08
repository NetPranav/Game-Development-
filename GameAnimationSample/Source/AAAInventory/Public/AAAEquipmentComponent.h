// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AAAItemTypes.h"
#include "AAAEquipmentComponent.generated.h"

class UAAAItemInstance;
class UAAAItemDefinition;
class UAAAInventoryComponent;

/* -------------------------------------------------------------------
 * Delegates
 * ------------------------------------------------------------------- */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAAAOnItemEquipped, UAAAItemInstance*, ItemInstance, EAAAEquipmentSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAAAOnItemUnequipped, UAAAItemInstance*, ItemInstance, EAAAEquipmentSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAAAOnEquipmentChanged);

USTRUCT(BlueprintType)
struct FAAAEquippedItemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AAA|Equipment")
	EAAAEquipmentSlot Slot = EAAAEquipmentSlot::None;

	UPROPERTY(BlueprintReadOnly, Category = "AAA|Equipment")
	TObjectPtr<UAAAItemInstance> ItemInstance = nullptr;
};

/**
 * UAAAEquipmentComponent
 *
 * Manages a character's equipped items across strongly typed slots.
 * Pure gameplay logic. No visuals, no mesh attachment, no animations.
 *
 * Responsibilities:
 * - Equip / Unequip / Swap items into typed slots
 * - Validate equipment requirements
 * - Notify owner of changes via delegates
 * - Replicate equipment state for multiplayer
 *
 * Equipment slots include standard RPG slots plus dragon-specific slots
 * for the mount system.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAAINVENTORY_API UAAAEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAAEquipmentComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent Interface

	/* -------------------------------------------------------------------
	 * Core API
	 * ------------------------------------------------------------------- */

	/**
	 * Equips an item into the appropriate slot (determined by the item's definition).
	 * If the slot is already occupied, the existing item is unequipped first.
	 * @param ItemInstance The item to equip.
	 * @return True if the item was successfully equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Equipment")
	bool EquipItem(UAAAItemInstance* ItemInstance);

	/**
	 * Equips an item into a specific slot override.
	 * @param ItemInstance The item to equip.
	 * @param Slot The target slot.
	 * @return True if the item was successfully equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Equipment")
	bool EquipItemToSlot(UAAAItemInstance* ItemInstance, EAAAEquipmentSlot Slot);

	/**
	 * Unequips whatever is in the given slot.
	 * @param Slot The slot to clear.
	 * @return The unequipped item instance, or nullptr if slot was empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Equipment")
	UAAAItemInstance* UnequipSlot(EAAAEquipmentSlot Slot);

	/**
	 * Swaps two equipment slots.
	 * @param SlotA First slot.
	 * @param SlotB Second slot.
	 * @return True if the swap succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Equipment")
	bool SwapSlots(EAAAEquipmentSlot SlotA, EAAAEquipmentSlot SlotB);

	/* -------------------------------------------------------------------
	 * Query API
	 * ------------------------------------------------------------------- */

	/** Returns the item currently equipped in a given slot. */
	UFUNCTION(BlueprintPure, Category = "AAA|Equipment")
	UAAAItemInstance* GetEquippedItem(EAAAEquipmentSlot Slot) const;

	/** Returns true if the given slot has an item equipped. */
	UFUNCTION(BlueprintPure, Category = "AAA|Equipment")
	bool IsSlotOccupied(EAAAEquipmentSlot Slot) const;

	/** Returns all currently equipped items as an array. */
	UFUNCTION(BlueprintPure, Category = "AAA|Equipment")
	TArray<UAAAItemInstance*> GetAllEquippedItems() const;

	/** Returns all occupied slots. */
	UFUNCTION(BlueprintPure, Category = "AAA|Equipment")
	TArray<EAAAEquipmentSlot> GetOccupiedSlots() const;

	/** Validates whether an item can be equipped to a specific slot. */
	UFUNCTION(BlueprintPure, Category = "AAA|Equipment")
	bool CanEquipToSlot(const UAAAItemInstance* ItemInstance, EAAAEquipmentSlot Slot) const;

	/* -------------------------------------------------------------------
	 * Delegates
	 * ------------------------------------------------------------------- */

	UPROPERTY(BlueprintAssignable, Category = "AAA|Equipment")
	FAAAOnItemEquipped OnItemEquipped;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Equipment")
	FAAAOnItemUnequipped OnItemUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Equipment")
	FAAAOnEquipmentChanged OnEquipmentChanged;

protected:
	/** Array of equipped items. Replicated. */
	UPROPERTY(Replicated, SaveGame)
	TArray<FAAAEquippedItemData> EquippedItems;
};
