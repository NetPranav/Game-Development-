// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "AAAItemTypes.h"
#include "AAAItemInstance.generated.h"

class UAAAItemDefinition;

/**
 * UAAAItemInstance
 *
 * Represents a single owned item in the game world.
 * This is the runtime counterpart to UAAAItemDefinition.
 *
 * While the Definition holds static, designer-authored data,
 * the Instance holds mutable, per-item state such as:
 * - Current stack count
 * - Current durability
 * - Quality roll
 * - Dynamic gameplay tags (enchantments, curses)
 * - A unique GUID for save/load and network identity
 *
 * Supports:
 * - Replication (via owning ActorComponent's replicated array)
 * - SaveGame serialization (all mutable fields are marked SaveGame)
 * - Unique identity via FGuid
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UAAAItemInstance();

	//~ Begin UObject Interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

	/* -------------------------------------------------------------------
	 * Static Reference (Points back to the Data Asset)
	 * ------------------------------------------------------------------- */

	/** The item archetype this instance was created from. */
	UPROPERTY(Replicated, BlueprintReadOnly, SaveGame, Category = "AAA|Item")
	TObjectPtr<UAAAItemDefinition> ItemDefinition;

	/* -------------------------------------------------------------------
	 * Mutable Runtime State
	 * ------------------------------------------------------------------- */

	/** Unique identifier for this specific item instance. Used for save/load and network sync. */
	UPROPERTY(Replicated, BlueprintReadOnly, SaveGame, Category = "AAA|Item")
	FGuid InstanceID;

	/** Current number of items in this stack. */
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "AAA|Item", meta = (ClampMin = "1"))
	int32 StackCount = 1;

	/** Current durability. Only meaningful if ItemDefinition->bHasDurability is true. */
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "AAA|Item", meta = (ClampMin = "0.0"))
	float CurrentDurability = 100.0f;

	/** Quality roll for this specific instance. */
	UPROPERTY(Replicated, BlueprintReadOnly, SaveGame, Category = "AAA|Item")
	EAAAItemQuality Quality = EAAAItemQuality::Normal;

	/** Dynamic tags applied to this instance (enchantments, curses, buffs). */
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "AAA|Item")
	FGameplayTagContainer DynamicTags;

	/** Weak reference to the owning actor (the character who has this in their inventory). */
	UPROPERTY(BlueprintReadOnly, Category = "AAA|Item")
	TWeakObjectPtr<AActor> OwnerActor;

	/* -------------------------------------------------------------------
	 * Convenience API
	 * ------------------------------------------------------------------- */

	/** Returns true if this instance has a valid definition. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	bool IsValid() const;

	/** Returns the display name from the definition. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	FText GetDisplayName() const;

	/** Returns the item category from the definition. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	EAAAItemCategory GetCategory() const;

	/** Returns true if the stack can accept more items. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	bool CanStack() const;

	/** Returns how many more items can be added to this stack. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	int32 GetRemainingStackSpace() const;

	/** Returns the total weight of this stack (Weight * StackCount). */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	float GetTotalWeight() const;

	/** Returns true if the item's durability is at zero. */
	UFUNCTION(BlueprintPure, Category = "AAA|Item")
	bool IsBroken() const;
};
