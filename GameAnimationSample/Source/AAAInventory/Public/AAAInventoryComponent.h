// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AAAItemTypes.h"
#include "AAAInventoryComponent.generated.h"

class UAAAItemInstance;
class UAAAItemDefinition;
class UAAAInventoryComponent;

/* ===================================================================
 * FAAAInventoryEntry — A single slot in the inventory
 *
 * Extends FFastArraySerializerItem so the net serializer can track
 * individual adds/removes/changes without replicating the whole array.
 * =================================================================== */
USTRUCT(BlueprintType)
struct FAAAInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAAAInventoryEntry() = default;

	/** The item instance occupying this slot. */
	UPROPERTY(BlueprintReadOnly, Category = "AAA|Inventory")
	TObjectPtr<UAAAItemInstance> Instance = nullptr;

	/** Convenience check */
	bool IsValid() const { return Instance != nullptr; }

	// FFastArraySerializerItem callbacks — forwarded to the owning list
	void PreReplicatedRemove(const struct FAAAInventoryList& InList);
	void PostReplicatedAdd(const struct FAAAInventoryList& InList);
	void PostReplicatedChange(const struct FAAAInventoryList& InList);
};

/* ===================================================================
 * FAAAInventoryList — The replicated inventory container
 *
 * Extends FFastArraySerializer to provide delta replication.
 * Only changed entries are serialized and sent over the network.
 * =================================================================== */
USTRUCT(BlueprintType)
struct FAAAInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FAAAInventoryList()
		: OwnerComponent(nullptr)
	{}

	/** The entries in the inventory. Delta-replicated via FFastArraySerializer. */
	UPROPERTY()
	TArray<FAAAInventoryEntry> Entries;

	/** Back-pointer to the owning component for delegate broadcasting. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UAAAInventoryComponent> OwnerComponent = nullptr;

	// FFastArraySerializer boilerplate — required for delta serialization
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAAAInventoryEntry, FAAAInventoryList>(Entries, DeltaParms, *this);
	}

	// Internal helpers used by UAAAInventoryComponent
	void AddEntry(UAAAItemInstance* NewInstance);
	void RemoveEntry(UAAAItemInstance* InstanceToRemove);
	FAAAInventoryEntry* FindEntry(UAAAItemInstance* Instance);
	FAAAInventoryEntry* FindEntryByDefinition(const UAAAItemDefinition* Definition);
};

/** Enable NetDeltaSerialize for FAAAInventoryList */
template<>
struct TStructOpsTypeTraits<FAAAInventoryList> : public TStructOpsTypeTraitsBase2<FAAAInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/* ===================================================================
 * Delegates
 * =================================================================== */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAAAOnItemAdded, UAAAItemInstance*, ItemInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAAAOnItemRemoved, UAAAItemInstance*, ItemInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAAAOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAAAOnStackChanged, UAAAItemInstance*, ItemInstance, int32, NewStackCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAAAOnDurabilityChanged, UAAAItemInstance*, ItemInstance, float, NewDurability);

/**
 * UAAAInventoryComponent
 *
 * Manages a character's item inventory. Pure gameplay logic.
 * No UI, no widgets, no drag-and-drop. Those belong to the AAAUI module.
 *
 * *** ARCHITECTURAL NOTE ***
 * Uses FFastArraySerializer for delta replication. Only changed items
 * are serialized and sent over the network, instead of replicating the
 * entire array on every mutation. This is the same pattern used by
 * Epic's Lyra project (ULyraInventoryManagerComponent).
 *
 * Responsibilities:
 * - Add / Remove / Move items
 * - Split and Merge stacks
 * - Find items by definition, tag, or GUID
 * - Track total weight
 * - Broadcast changes via delegates
 *
 * Fully replicated for multiplayer. Authority-validated.
 * All mutable state is SaveGame-compatible.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAAINVENTORY_API UAAAInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAAInventoryComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~ End UActorComponent Interface

	/* -------------------------------------------------------------------
	 * Configuration
	 * ------------------------------------------------------------------- */

	/** Maximum number of item slots in this inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Inventory", meta = (ClampMin = "1"))
	int32 MaxSlots = 40;

	/** Maximum total weight this inventory can hold. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Inventory", meta = (ClampMin = "0.0"))
	float MaxWeight = 0.0f;

	/* -------------------------------------------------------------------
	 * Core API
	 * ------------------------------------------------------------------- */

	/**
	 * Creates a new ItemInstance from a Definition and adds it to the inventory.
	 * Handles stacking automatically.
	 * @param Definition The item archetype to add.
	 * @param Count Number of items to add.
	 * @return The ItemInstance that was created/stacked into, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	UAAAItemInstance* AddItem(UAAAItemDefinition* Definition, int32 Count = 1);

	/**
	 * Adds an existing ItemInstance to the inventory.
	 * @param ItemInstance The instance to add.
	 * @return True if the item was successfully added.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	bool AddItemInstance(UAAAItemInstance* ItemInstance);

	/**
	 * Removes an item instance from the inventory entirely.
	 * @param ItemInstance The instance to remove.
	 * @return True if the item was found and removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	bool RemoveItem(UAAAItemInstance* ItemInstance);

	/**
	 * Removes a specific count from a stack. If count >= stack count, removes the item.
	 * @param ItemInstance The instance to reduce.
	 * @param Count Number of items to remove from the stack.
	 * @return True if the operation succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	bool RemoveItemCount(UAAAItemInstance* ItemInstance, int32 Count = 1);

	/**
	 * Consumes (uses) an item, decrementing its stack and removing if empty.
	 * @param ItemInstance The consumable instance.
	 * @return True if the item was consumed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	bool ConsumeItem(UAAAItemInstance* ItemInstance);

	/**
	 * Splits a stack into two, creating a new ItemInstance with SplitCount items.
	 * @param ItemInstance The source stack.
	 * @param SplitCount How many items to split off.
	 * @return The new ItemInstance, or nullptr if split failed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	UAAAItemInstance* SplitStack(UAAAItemInstance* ItemInstance, int32 SplitCount);

	/**
	 * Merges Source stack into Target stack (if same item type).
	 * @param Source The stack to merge from (will be removed if fully merged).
	 * @param Target The stack to merge into.
	 * @return True if any items were merged.
	 */
	UFUNCTION(BlueprintCallable, Category = "AAA|Inventory")
	bool MergeStacks(UAAAItemInstance* Source, UAAAItemInstance* Target);

	/* -------------------------------------------------------------------
	 * Query API
	 * ------------------------------------------------------------------- */

	/** Finds the first item instance matching the given definition. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	UAAAItemInstance* FindItemByDefinition(const UAAAItemDefinition* Definition) const;

	/** Finds an item instance by its unique GUID. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	UAAAItemInstance* FindItemByGUID(const FGuid& InstanceID) const;

	/** Returns all item instances matching a gameplay tag query. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	TArray<UAAAItemInstance*> FindItemsByTag(const FGameplayTag& Tag) const;

	/** Returns true if the inventory contains at least Count of the given definition. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	bool HasItem(const UAAAItemDefinition* Definition, int32 Count = 1) const;

	/** Returns the total count of a specific item definition across all stacks. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	int32 GetItemCount(const UAAAItemDefinition* Definition) const;

	/** Returns the total weight of all items in the inventory. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	float GetTotalWeight() const;

	/** Returns the number of occupied slots. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	int32 GetOccupiedSlots() const;

	/** Returns true if there is at least one empty slot. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	bool HasEmptySlot() const;

	/** Returns all item instances. */
	UFUNCTION(BlueprintPure, Category = "AAA|Inventory")
	TArray<UAAAItemInstance*> GetAllItems() const;

	/* -------------------------------------------------------------------
	 * Delegates
	 * ------------------------------------------------------------------- */

	UPROPERTY(BlueprintAssignable, Category = "AAA|Inventory")
	FAAAOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Inventory")
	FAAAOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Inventory")
	FAAAOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Inventory")
	FAAAOnStackChanged OnStackChanged;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Inventory")
	FAAAOnDurabilityChanged OnDurabilityChanged;

	// Allow FAAAInventoryList to access private members
	friend struct FAAAInventoryList;

protected:
	/** The delta-replicated inventory list. Only changed entries are sent over the wire. */
	UPROPERTY(Replicated, SaveGame)
	FAAAInventoryList InventoryList;

	/** Internal helper: checks weight and slot capacity before adding. */
	bool CanAddItem(const UAAAItemDefinition* Definition, int32 Count) const;
};
