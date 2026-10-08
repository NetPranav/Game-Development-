// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAAAbilitySettings.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FAAAAbilityUIMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FLinearColor AbilityThemeColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FAAAAbilityExecutionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TObjectPtr<UAnimMontage> CastingAnimation;
};

USTRUCT(BlueprintType)
struct FAAAAbilityCostAndCooldown
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	TSubclassOf<UGameplayEffect> CooldownEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	TSubclassOf<UGameplayEffect> CostEffect;

	/** Explicit base cooldown in seconds (for UI display without instantiating the GE) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	float BaseCooldown = 0.0f;

	/** Explicit base stamina/mana cost (for UI display) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	float BaseCost = 0.0f;
};

/**
 * Primary Data Asset holding all configuration and UI data for a single gameplay ability.
 * Allows the UI to build spell books and action bars without hardcoded lookups.
 */
UCLASS(BlueprintType)
class AAAABILITY_API UAAAAbilitySettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Ability")
	FAAAAbilityUIMetadata UIMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Ability")
	FAAAAbilityExecutionSettings Execution;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Ability")
	FAAAAbilityCostAndCooldown Requirements;

	/** Tags that dictate if the ability can even be slotted (e.g., Requires.Weapon.Bow) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Ability")
	FGameplayTagContainer RequirementTags;
};
