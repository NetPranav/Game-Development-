// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AAAAPlayerPawn.generated.h"

// Forward Declarations
class UAAAAbilitySystemComponent;
class UAAAAttributeSet;
class UAAAAttributeComponent;
class UAAATeamComponent;
class UAAAReplicationComponent;
class UAAAInventoryComponent;
class UAAAEquipmentComponent;
class UAAAInteractionComponent;
class UAAACombatComponent;
class UAAAAbilityInputBindingComponent;
class UAAACharacterSettings;

/**
 * The core Pawn class for the AAA Framework.
 * Inherits from APawn to remain perfectly compatible with Epic's Mover plugin.
 * Acts strictly as a composition root for gameplay components.
 *
 * *** ARCHITECTURAL NOTE (Lyra Pattern) ***
 * The ASC and AttributeSet are NOT owned by this Pawn.
 * They live on AAAAAPlayerState to survive pawn death/respawn.
 * This Pawn caches weak pointers to the PlayerState's ASC and serves
 * as the "Avatar Actor" for GAS (InitAbilityActorInfo(PS, this)).
 *
 * For AI-controlled pawns (enemies, NPCs), the ASC lives directly on
 * the Pawn since they don't have a PlayerState.
 */
UCLASS(Abstract, Blueprintable)
class AAACHARACTER_API AAAAAPlayerPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAAAAPlayerPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

protected:
	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PawnClientRestart() override;
	//~ End AActor Interface

	/**
	 * Fetches the ASC from the PlayerState and initializes Actor Info.
	 * Called from both PossessedBy (server) and OnRep_PlayerState (client).
	 */
	virtual void InitAbilityActorInfoFromPlayerState();

	/** Fired when the AttributeComponent broadcasts death */
	UFUNCTION()
	virtual void OnDeath();

protected:
	/* -------------------------------------------------------------------
	 * Cached Ability System Pointers (Owned by PlayerState)
	 *
	 * These are fetched during PossessedBy/OnRep_PlayerState and cached
	 * for fast access. They are NOT owned by this Pawn.
	 * ------------------------------------------------------------------- */

	UPROPERTY(Transient, BlueprintReadOnly, Category = "AAA|Abilities")
	TWeakObjectPtr<UAAAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "AAA|Abilities")
	TWeakObjectPtr<const UAAAAttributeSet> AttributeSet;

	/* -------------------------------------------------------------------
	 * Core Gameplay Components (Owned by this Pawn)
	 * ------------------------------------------------------------------- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Attributes")
	TObjectPtr<UAAAAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Team")
	TObjectPtr<UAAATeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Replication")
	TObjectPtr<UAAAReplicationComponent> ReplicationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Inventory")
	TObjectPtr<UAAAInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Equipment")
	TObjectPtr<UAAAEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Interaction")
	TObjectPtr<UAAAInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Combat")
	TObjectPtr<UAAACombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Input")
	TObjectPtr<UAAAAbilityInputBindingComponent> AbilityInputBindingComponent;

	/* -------------------------------------------------------------------
	 * Data Driven Configuration
	 * ------------------------------------------------------------------- */

	/** Data Asset defining character settings (movement, input, etc.) without hardcoding */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings")
	TObjectPtr<UAAACharacterSettings> CharacterSettings;
};

// Force Recompile
