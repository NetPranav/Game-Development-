// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAAAbilityInputBindingComponent.h"
#include "AAACharacterSettings.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UInputMappingContext;

/* -------------------------------------------------------------------
 * Movement Configuration
 * ------------------------------------------------------------------- */
USTRUCT(BlueprintType)
struct FAAACharacterMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Acceleration = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float JumpForce = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float AirControl = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RotationRate = 500.0f;
};

/* -------------------------------------------------------------------
 * Vital Configuration (Health, Stamina)
 * ------------------------------------------------------------------- */
USTRUCT(BlueprintType)
struct FAAACharacterVitalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
	float BaseHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
	float BaseStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
	float BaseMovementSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vitals")
	float RespawnDelay = 5.0f;
};

/* -------------------------------------------------------------------
 * Camera Configuration
 * ------------------------------------------------------------------- */
USTRUCT(BlueprintType)
struct FAAACharacterCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float DefaultTargetArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector SocketOffset = FVector(0, 50, 50);
};

/* -------------------------------------------------------------------
 * Input Configuration
 * ------------------------------------------------------------------- */

USTRUCT(BlueprintType)
struct FAAACharacterInputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/**
	 * Tag-based ability input bindings (Lyra pattern).
	 * Maps Enhanced Input Actions → Gameplay Tags → ASC ability activation.
	 * Designers configure this in the Data Asset — no C++ changes needed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Abilities")
	TArray<FAAAAbilityInputBinding> AbilityInputBindings;
};

/* -------------------------------------------------------------------
 * Ability System Configuration
 * ------------------------------------------------------------------- */
USTRUCT(BlueprintType)
struct FAAACharacterAbilitySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer StartingTags;
};

/**
 * Primary Data Asset holding all configuration values for a character.
 * Instanced per character type to prevent hardcoding values in C++.
 */
UCLASS(BlueprintType)
class AAACHARACTER_API UAAACharacterSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings|Vitals")
	FAAACharacterVitalSettings Vitals;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings|Movement")
	FAAACharacterMovementSettings Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings|Abilities")
	FAAACharacterAbilitySettings Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings|Camera")
	FAAACharacterCameraSettings Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Settings|Input")
	FAAACharacterInputSettings Input;
};
