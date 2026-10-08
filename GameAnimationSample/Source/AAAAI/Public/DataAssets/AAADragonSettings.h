// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAADragonSettings.generated.h"

class UBehaviorTree;
class UGameplayAbility;
class UAnimSequence;
class UAnimMontage;

UENUM(BlueprintType)
enum class EAAADragonElement : uint8
{
	Fire	UMETA(DisplayName = "Fire"),
	Ice		UMETA(DisplayName = "Ice"),
	Lightning UMETA(DisplayName = "Lightning"),
	Poison	UMETA(DisplayName = "Poison"),
	Arcane	UMETA(DisplayName = "Arcane")
};

UENUM(BlueprintType)
enum class EAAADragonGrowthStage : uint8
{
	Hatchling	UMETA(DisplayName = "Hatchling"),
	Drake		UMETA(DisplayName = "Drake"),
	Adult		UMETA(DisplayName = "Adult"),
	Elder		UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FAAADragonFlightSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float MaxFlightSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlightAcceleration = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float TakeoffStaminaCost = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float StaminaDrainPerSecond = 5.0f;
};

USTRUCT(BlueprintType)
struct FAAADragonMountSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	bool bIsRideable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	FName RiderSocketName = FName("RiderSeat");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TObjectPtr<UAnimMontage> MountAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TObjectPtr<UAnimMontage> DismountAnimation;
};

/**
 * Primary Data Asset holding all configuration for Dragon entities.
 */
UCLASS(BlueprintType)
class AAAAI_API UAAADragonSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Identity")
	EAAADragonElement ElementType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Identity")
	EAAADragonGrowthStage GrowthStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Vitals")
	float BaseHealth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Vitals")
	float BaseStamina = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Flight")
	FAAADragonFlightSettings Flight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Mounting")
	FAAADragonMountSettings Mount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Combat")
	TArray<TSubclassOf<UGameplayAbility>> InnateAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Dragon|Tags")
	FGameplayTagContainer DescriptiveTags;
};
