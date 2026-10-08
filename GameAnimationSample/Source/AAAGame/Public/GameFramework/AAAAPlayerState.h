// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AAAAPlayerState.generated.h"

class UAAAAbilitySystemComponent;
class UAAAAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

/**
 * AAAAAPlayerState
 * 
 * Persistent player data that survives pawn possession and death.
 * Replicated to all clients.
 * 
 * *** ARCHITECTURAL DECISION (Lyra/Fortnite Pattern) ***
 * The AbilitySystemComponent and AttributeSet live HERE on the PlayerState,
 * NOT on the Pawn. This ensures all active GameplayEffects, granted abilities,
 * cooldowns, and tags persist across pawn death and respawn.
 * 
 * The Pawn (AAAAPlayerPawn) is the "Avatar" — it caches a pointer to this
 * PlayerState's ASC via InitAbilityActorInfo(PlayerState, Pawn).
 * 
 * Responsibilities:
 * - Ability System Component ownership (GAS lifecycle)
 * - Core Attribute Set ownership (Health, Stamina, etc.)
 * - Level, Experience, Gold
 * - Reputation & Dragon Bonds
 * - Playtime & Achievement Progress
 * - Guild / Party Information
 */
UCLASS()
class AAAGAME_API AAAAAPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAAAAPlayerState();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	/** Returns the typed AAA Ability System Component. */
	UFUNCTION(BlueprintPure, Category = "AAA|Abilities")
	UAAAAbilitySystemComponent* GetAAAAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Returns the core Attribute Set. */
	UFUNCTION(BlueprintPure, Category = "AAA|Abilities")
	UAAAAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/**
	 * Grants startup abilities, effects, and tags.
	 * Called by the Pawn when possessed. Safe to call only on Authority.
	 * Accepts raw arrays so this class has NO dependency on AAACharacter module.
	 */
	void GrantStartupAbilities(
		const TArray<TSubclassOf<UGameplayAbility>>& InAbilities,
		const TArray<TSubclassOf<UGameplayEffect>>& InEffects,
		const FGameplayTagContainer& InStartingTags
	);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* -------------------------------------------------------------------
	 * Ability System — Owned by PlayerState (Lyra Pattern)
	 * ------------------------------------------------------------------- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Abilities")
	TObjectPtr<UAAAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAAAAttributeSet> AttributeSet;

	/** Guard flag to prevent double-granting abilities on reconnect. */
	bool bAbilitiesGranted = false;

	//-------------------------------------------------------------------------
	// Future Expansion Points
	//-------------------------------------------------------------------------
	
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AAA|Progression")
	// int32 PlayerLevel;
	
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AAA|Progression")
	// float PlayerExperience;
	
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AAA|Economy")
	// int32 Gold;
};
