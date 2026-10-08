// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AAAAbilitySystemComponent.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * Custom Ability System Component for the AAA Framework.
 * Serves as the core orchestrator for gameplay abilities, attributes, and tags.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAAABILITY_API UAAAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAAAAbilitySystemComponent();

protected:
	virtual void BeginPlay() override;

	bool bCharacterAbilitiesGiven;

public:
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
	/** Hook for when abilities are granted and the ASC is ready. */
	virtual void OnAbilitySystemInitialized();

	/** Grants startup abilities and effects from an external data source. Safe to call only on Authority. */
	void GrantStartupData(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities, const TArray<TSubclassOf<UGameplayEffect>>& InEffects);
};
