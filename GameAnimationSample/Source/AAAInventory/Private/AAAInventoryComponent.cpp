// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAInventoryComponent.h"
#include "AAAItemInstance.h"
#include "AAAItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

// ===================================================================
// FAAAInventoryEntry — FFastArraySerializerItem callbacks
// ===================================================================

void FAAAInventoryEntry::PreReplicatedRemove(const FAAAInventoryList& InList)
{
	if (InList.OwnerComponent && Instance)
	{
		InList.OwnerComponent->OnItemRemoved.Broadcast(Instance);
		InList.OwnerComponent->OnInventoryChanged.Broadcast();
	}
}

void FAAAInventoryEntry::PostReplicatedAdd(const FAAAInventoryList& InList)
{
	if (InList.OwnerComponent && Instance)
	{
		InList.OwnerComponent->OnItemAdded.Broadcast(Instance);
		InList.OwnerComponent->OnInventoryChanged.Broadcast();
	}
}

void FAAAInventoryEntry::PostReplicatedChange(const FAAAInventoryList& InList)
{
	if (InList.OwnerComponent)
	{
		InList.OwnerComponent->OnInventoryChanged.Broadcast();
	}
}

// ===================================================================
// FAAAInventoryList — Internal helpers
// ===================================================================

void FAAAInventoryList::AddEntry(UAAAItemInstance* NewInstance)
{
	if (!NewInstance) return;

	FAAAInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewInstance;
	MarkItemDirty(NewEntry);
}

void FAAAInventoryList::RemoveEntry(UAAAItemInstance* InstanceToRemove)
{
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (Entries[i].Instance == InstanceToRemove)
		{
			Entries.RemoveAt(i);
			MarkArrayDirty();
			return;
		}
	}
}

FAAAInventoryEntry* FAAAInventoryList::FindEntry(UAAAItemInstance* Instance)
{
	for (FAAAInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance == Instance)
		{
			return &Entry;
		}
	}
	return nullptr;
}

FAAAInventoryEntry* FAAAInventoryList::FindEntryByDefinition(const UAAAItemDefinition* Definition)
{
	for (FAAAInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDefinition == Definition)
		{
			return &Entry;
		}
	}
	return nullptr;
}

// ===================================================================
// UAAAInventoryComponent
// ===================================================================

UAAAInventoryComponent::UAAAInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAAAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set the back-pointer so FAAAInventoryEntry callbacks can broadcast delegates
	InventoryList.OwnerComponent = this;
}

void UAAAInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAAAInventoryComponent, InventoryList);
}

bool UAAAInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Replicate item instances as subobjects so they serialize properly
	for (FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			bWroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

// -------------------------------------------------------------------
// Core API
// -------------------------------------------------------------------

bool UAAAInventoryComponent::CanAddItem(const UAAAItemDefinition* Definition, int32 Count) const
{
	if (!Definition || Count <= 0) return false;

	// Check weight limit
	if (MaxWeight > 0.0f)
	{
		float AddedWeight = Definition->Weight * static_cast<float>(Count);
		if (GetTotalWeight() + AddedWeight > MaxWeight)
		{
			return false;
		}
	}

	// Check if we can stack into an existing slot
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDefinition == Definition && Entry.Instance->CanStack())
		{
			return true;
		}
	}

	// Check if we have an empty slot
	return GetOccupiedSlots() < MaxSlots;
}

UAAAItemInstance* UAAAInventoryComponent::AddItem(UAAAItemDefinition* Definition, int32 Count)
{
	if (!Definition || Count <= 0) return nullptr;
	if (!GetOwner()->HasAuthority()) return nullptr;
	if (!CanAddItem(Definition, Count)) return nullptr;

	// Try to stack into an existing item first
	for (FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDefinition == Definition && Entry.Instance->CanStack())
		{
			int32 SpaceAvailable = Entry.Instance->GetRemainingStackSpace();
			int32 ToAdd = FMath::Min(Count, SpaceAvailable);
			Entry.Instance->StackCount += ToAdd;
			Count -= ToAdd;

			// Mark this specific entry as dirty — only this entry replicates
			InventoryList.MarkItemDirty(Entry);

			OnStackChanged.Broadcast(Entry.Instance, Entry.Instance->StackCount);
			OnInventoryChanged.Broadcast();

			if (Count <= 0)
			{
				return Entry.Instance;
			}
		}
	}

	// Create new instance(s) for remaining count
	UAAAItemInstance* LastCreated = nullptr;
	while (Count > 0 && GetOccupiedSlots() < MaxSlots)
	{
		UAAAItemInstance* NewInstance = NewObject<UAAAItemInstance>(GetOwner());
		NewInstance->ItemDefinition = Definition;
		NewInstance->StackCount = FMath::Min(Count, Definition->MaxStackSize);
		NewInstance->OwnerActor = GetOwner();

		if (Definition->bHasDurability)
		{
			NewInstance->CurrentDurability = Definition->MaxDurability;
		}

		Count -= NewInstance->StackCount;

		// Add via the list helper — marks dirty automatically
		InventoryList.AddEntry(NewInstance);
		LastCreated = NewInstance;

		OnItemAdded.Broadcast(NewInstance);
	}

	OnInventoryChanged.Broadcast();
	return LastCreated;
}

bool UAAAInventoryComponent::AddItemInstance(UAAAItemInstance* ItemInstance)
{
	if (!ItemInstance || !ItemInstance->IsValid()) return false;
	if (!GetOwner()->HasAuthority()) return false;
	if (GetOccupiedSlots() >= MaxSlots) return false;

	ItemInstance->OwnerActor = GetOwner();
	InventoryList.AddEntry(ItemInstance);

	OnItemAdded.Broadcast(ItemInstance);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UAAAInventoryComponent::RemoveItem(UAAAItemInstance* ItemInstance)
{
	if (!ItemInstance) return false;
	if (!GetOwner()->HasAuthority()) return false;

	FAAAInventoryEntry* Entry = InventoryList.FindEntry(ItemInstance);
	if (!Entry) return false;

	ItemInstance->OwnerActor = nullptr;
	InventoryList.RemoveEntry(ItemInstance);

	OnItemRemoved.Broadcast(ItemInstance);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UAAAInventoryComponent::RemoveItemCount(UAAAItemInstance* ItemInstance, int32 Count)
{
	if (!ItemInstance || Count <= 0) return false;
	if (!GetOwner()->HasAuthority()) return false;

	FAAAInventoryEntry* Entry = InventoryList.FindEntry(ItemInstance);
	if (!Entry) return false;

	if (Count >= ItemInstance->StackCount)
	{
		return RemoveItem(ItemInstance);
	}

	ItemInstance->StackCount -= Count;
	InventoryList.MarkItemDirty(*Entry);

	OnStackChanged.Broadcast(ItemInstance, ItemInstance->StackCount);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UAAAInventoryComponent::ConsumeItem(UAAAItemInstance* ItemInstance)
{
	return RemoveItemCount(ItemInstance, 1);
}

UAAAItemInstance* UAAAInventoryComponent::SplitStack(UAAAItemInstance* ItemInstance, int32 SplitCount)
{
	if (!ItemInstance || SplitCount <= 0) return nullptr;
	if (!GetOwner()->HasAuthority()) return nullptr;
	if (SplitCount >= ItemInstance->StackCount) return nullptr;
	if (GetOccupiedSlots() >= MaxSlots) return nullptr;

	// Create new instance with split items
	UAAAItemInstance* NewInstance = NewObject<UAAAItemInstance>(GetOwner());
	NewInstance->ItemDefinition = ItemInstance->ItemDefinition;
	NewInstance->StackCount = SplitCount;
	NewInstance->Quality = ItemInstance->Quality;
	NewInstance->CurrentDurability = ItemInstance->CurrentDurability;
	NewInstance->DynamicTags = ItemInstance->DynamicTags;
	NewInstance->OwnerActor = GetOwner();

	// Reduce original stack
	ItemInstance->StackCount -= SplitCount;

	// Mark the original entry dirty
	FAAAInventoryEntry* OriginalEntry = InventoryList.FindEntry(ItemInstance);
	if (OriginalEntry)
	{
		InventoryList.MarkItemDirty(*OriginalEntry);
	}

	// Add the new entry
	InventoryList.AddEntry(NewInstance);

	OnStackChanged.Broadcast(ItemInstance, ItemInstance->StackCount);
	OnItemAdded.Broadcast(NewInstance);
	OnInventoryChanged.Broadcast();

	return NewInstance;
}

bool UAAAInventoryComponent::MergeStacks(UAAAItemInstance* Source, UAAAItemInstance* Target)
{
	if (!Source || !Target) return false;
	if (!GetOwner()->HasAuthority()) return false;
	if (Source == Target) return false;
	if (!Source->IsValid() || !Target->IsValid()) return false;
	if (Source->ItemDefinition != Target->ItemDefinition) return false;
	if (!Target->CanStack()) return false;

	int32 SpaceAvailable = Target->GetRemainingStackSpace();
	int32 ToMerge = FMath::Min(Source->StackCount, SpaceAvailable);

	if (ToMerge <= 0) return false;

	Target->StackCount += ToMerge;
	Source->StackCount -= ToMerge;

	// Mark target dirty
	FAAAInventoryEntry* TargetEntry = InventoryList.FindEntry(Target);
	if (TargetEntry)
	{
		InventoryList.MarkItemDirty(*TargetEntry);
	}

	OnStackChanged.Broadcast(Target, Target->StackCount);

	if (Source->StackCount <= 0)
	{
		RemoveItem(Source);
	}
	else
	{
		FAAAInventoryEntry* SourceEntry = InventoryList.FindEntry(Source);
		if (SourceEntry)
		{
			InventoryList.MarkItemDirty(*SourceEntry);
		}
		OnStackChanged.Broadcast(Source, Source->StackCount);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

// -------------------------------------------------------------------
// Query API
// -------------------------------------------------------------------

UAAAItemInstance* UAAAInventoryComponent::FindItemByDefinition(const UAAAItemDefinition* Definition) const
{
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDefinition == Definition)
		{
			return Entry.Instance;
		}
	}
	return nullptr;
}

UAAAItemInstance* UAAAInventoryComponent::FindItemByGUID(const FGuid& InstanceID) const
{
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->InstanceID == InstanceID)
		{
			return Entry.Instance;
		}
	}
	return nullptr;
}

TArray<UAAAItemInstance*> UAAAInventoryComponent::FindItemsByTag(const FGameplayTag& Tag) const
{
	TArray<UAAAItemInstance*> Results;
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->IsValid())
		{
			// Check both static definition tags and dynamic instance tags
			if (Entry.Instance->ItemDefinition->ItemTags.HasTag(Tag) || Entry.Instance->DynamicTags.HasTag(Tag))
			{
				Results.Add(Entry.Instance);
			}
		}
	}
	return Results;
}

bool UAAAInventoryComponent::HasItem(const UAAAItemDefinition* Definition, int32 Count) const
{
	return GetItemCount(Definition) >= Count;
}

int32 UAAAInventoryComponent::GetItemCount(const UAAAItemDefinition* Definition) const
{
	int32 Total = 0;
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDefinition == Definition)
		{
			Total += Entry.Instance->StackCount;
		}
	}
	return Total;
}

float UAAAInventoryComponent::GetTotalWeight() const
{
	float Total = 0.0f;
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			Total += Entry.Instance->GetTotalWeight();
		}
	}
	return Total;
}

int32 UAAAInventoryComponent::GetOccupiedSlots() const
{
	return InventoryList.Entries.Num();
}

bool UAAAInventoryComponent::HasEmptySlot() const
{
	return GetOccupiedSlots() < MaxSlots;
}

TArray<UAAAItemInstance*> UAAAInventoryComponent::GetAllItems() const
{
	TArray<UAAAItemInstance*> Results;
	Results.Reserve(InventoryList.Entries.Num());
	for (const FAAAInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}
