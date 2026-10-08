// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAAFactionSettings.generated.h"

class UTexture2D;

/**
 * Primary Data Asset holding relationship and identity configuration for a Faction.
 */
UCLASS(BlueprintType)
class AAACORE_API UAAAFactionSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Identity")
	FGameplayTag FactionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Identity")
	FText FactionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Identity")
	TObjectPtr<UTexture2D> FactionBanner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Relationships")
	FGameplayTagContainer FriendlyFactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Relationships")
	FGameplayTagContainer EnemyFactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Faction|Relationships")
	float StartingReputation = 0.0f;
};
