// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAEquipmentComponent.h"
#include "AAAItemInstance.h"
#include "AAAItemDefinition.h"
#include "Net/UnrealNetwork.h"

UAAAEquipmentComponent::UAAAEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAAAEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAAAEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAAAEquipmentComponent, EquippedItems);
}

// -------------------------------------------------------------------
// Core API
// -------------------------------------------------------------------

bool UAAAEquipmentComponent::EquipItem(UAAAItemInstance* ItemInstance)
{
	if (!ItemInstance || !ItemInstance->IsValid()) return false;

	EAAAEquipmentSlot TargetSlot = ItemInstance->ItemDefinition->EquipmentSlot;
	if (TargetSlot == EAAAEquipmentSlot::None) return false;

	return EquipItemToSlot(ItemInstance, TargetSlot);
}

bool UAAAEquipmentComponent::EquipItemToSlot(UAAAItemInstance* ItemInstance, EAAAEquipmentSlot Slot)
{
	if (!ItemInstance || !ItemInstance->IsValid()) return false;
	if (Slot == EAAAEquipmentSlot::None) return false;
	if (!GetOwner()->HasAuthority()) return false;
	if (!CanEquipToSlot(ItemInstance, Slot)) return false;

	// Unequip existing item in the slot if occupied
	if (IsSlotOccupied(Slot))
	{
		UnequipSlot(Slot);
	}

	FAAAEquippedItemData NewData;
	NewData.Slot = Slot;
	NewData.ItemInstance = ItemInstance;
	EquippedItems.Add(NewData);

	OnItemEquipped.Broadcast(ItemInstance, Slot);
	OnEquipmentChanged.Broadcast();

	return true;
}

UAAAItemInstance* UAAAEquipmentComponent::UnequipSlot(EAAAEquipmentSlot Slot)
{
	if (Slot == EAAAEquipmentSlot::None) return nullptr;
	if (!GetOwner()->HasAuthority()) return nullptr;

	for (int32 i = 0; i < EquippedItems.Num(); ++i)
	{
		if (EquippedItems[i].Slot == Slot)
		{
			UAAAItemInstance* UnequippedItem = EquippedItems[i].ItemInstance;
			EquippedItems.RemoveAt(i);

			OnItemUnequipped.Broadcast(UnequippedItem, Slot);
			OnEquipmentChanged.Broadcast();

			return UnequippedItem;
		}
	}

	return nullptr;
}

bool UAAAEquipmentComponent::SwapSlots(EAAAEquipmentSlot SlotA, EAAAEquipmentSlot SlotB)
{
	if (SlotA == EAAAEquipmentSlot::None || SlotB == EAAAEquipmentSlot::None) return false;
	if (SlotA == SlotB) return false;
	if (!GetOwner()->HasAuthority()) return false;

	UAAAItemInstance* ItemA = GetEquippedItem(SlotA);
	UAAAItemInstance* ItemB = GetEquippedItem(SlotB);

	// Clear both slots
	if (ItemA) UnequipSlot(SlotA);
	if (ItemB) UnequipSlot(SlotB);

	// Swap
	if (ItemA) EquipItemToSlot(ItemA, SlotB);
	if (ItemB) EquipItemToSlot(ItemB, SlotA);

	OnEquipmentChanged.Broadcast();
	return true;
}

// -------------------------------------------------------------------
// Query API
// -------------------------------------------------------------------

UAAAItemInstance* UAAAEquipmentComponent::GetEquippedItem(EAAAEquipmentSlot Slot) const
{
	for (const FAAAEquippedItemData& Data : EquippedItems)
	{
		if (Data.Slot == Slot)
		{
			return Data.ItemInstance;
		}
	}
	return nullptr;
}

bool UAAAEquipmentComponent::IsSlotOccupied(EAAAEquipmentSlot Slot) const
{
	return GetEquippedItem(Slot) != nullptr;
}

TArray<UAAAItemInstance*> UAAAEquipmentComponent::GetAllEquippedItems() const
{
	TArray<UAAAItemInstance*> Results;
	for (const FAAAEquippedItemData& Data : EquippedItems)
	{
		if (Data.ItemInstance)
		{
			Results.Add(Data.ItemInstance);
		}
	}
	return Results;
}

TArray<EAAAEquipmentSlot> UAAAEquipmentComponent::GetOccupiedSlots() const
{
	TArray<EAAAEquipmentSlot> Results;
	for (const FAAAEquippedItemData& Data : EquippedItems)
	{
		if (Data.ItemInstance)
		{
			Results.Add(Data.Slot);
		}
	}
	return Results;
}

bool UAAAEquipmentComponent::CanEquipToSlot(const UAAAItemInstance* ItemInstance, EAAAEquipmentSlot Slot) const
{
	if (!ItemInstance || !ItemInstance->IsValid()) return false;
	if (Slot == EAAAEquipmentSlot::None) return false;

	// Basic validation: does the item's definition allow this slot?
	const UAAAItemDefinition* Def = ItemInstance->ItemDefinition;
	if (Def->EquipmentSlot != Slot)
	{
		// Allow rings to go in either ring slot
		if (Def->EquipmentSlot == EAAAEquipmentSlot::RingLeft && Slot == EAAAEquipmentSlot::RingRight) return true;
		if (Def->EquipmentSlot == EAAAEquipmentSlot::RingRight && Slot == EAAAEquipmentSlot::RingLeft) return true;

		return false;
	}

	return true;
}
