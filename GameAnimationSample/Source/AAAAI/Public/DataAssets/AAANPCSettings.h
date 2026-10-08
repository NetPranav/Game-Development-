// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAANPCSettings.generated.h"

class UBehaviorTree;
class UGameplayAbility;
class USoundCue;

USTRUCT(BlueprintType)
struct FAAANPCCombatSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<TSubclassOf<UGameplayAbility>> NPCAbilities;
};

USTRUCT(BlueprintType)
struct FAAANPCProgressionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float XPGrantedOnDeath = 50.0f;

	// Note: Replace UObject with your actual LootTable class in the future
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TObjectPtr<UObject> LootTable; 
};

USTRUCT(BlueprintType)
struct FAAANPCAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> AggroVoice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> DeathVoice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> IdleVoice;
};

/**
 * Primary Data Asset holding all configuration for a specific NPC type (Bandit, Guard, Merchant).
 */
UCLASS(BlueprintType)
class AAAAI_API UAAANPCSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC|Core")
	FGameplayTag FactionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC|Core")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC")
	FAAANPCCombatSettings Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC")
	FAAANPCProgressionSettings Progression;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC")
	FAAANPCAudioSettings Audio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|NPC")
	FGameplayTagContainer DescriptiveTags;
};
