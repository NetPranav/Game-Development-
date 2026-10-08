// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AAAWeaponSettings.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UAnimMontage;
class UGameplayAbility;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EAAAWeaponType : uint8
{
	Unarmed			UMETA(DisplayName = "Unarmed"),
	OneHandedSword	UMETA(DisplayName = "One-Handed Sword"),
	TwoHandedSword	UMETA(DisplayName = "Two-Handed Sword"),
	Bow				UMETA(DisplayName = "Bow"),
	Staff			UMETA(DisplayName = "Magic Staff"),
	Shield			UMETA(DisplayName = "Shield")
};

USTRUCT(BlueprintType)
struct FAAAWeaponVisualSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FName AttachmentSocket = FName("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FName HolsterSocket = FName("spine_03");

	/** Weapon actor transform relative to the hand socket (tune per mesh: sword blades rarely line up with the socket axes). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FTransform HandOffset = FTransform::Identity;

	/** Weapon actor transform relative to the holster socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FTransform HolsterOffset = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FAAAWeaponCombatSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float StaminaCostPerSwing = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Weight = 5.0f;
};

USTRUCT(BlueprintType)
struct FAAAWeaponAbilitySettings
{
	GENERATED_BODY()

	/** Abilities granted to the player while this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	/** Passive effects applied while equipped (e.g. +Strength buff) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** Tags added to the player while equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer WeaponTags;
};

USTRUCT(BlueprintType)
struct FAAAWeaponAnimationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimMontage>> LightAttackCombos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimMontage>> HeavyAttackCombos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> UnequipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ParryMontage;
};

/**
 * Primary Data Asset holding all configuration values for a specific weapon.
 */
UCLASS(BlueprintType)
class AAACOMBAT_API UAAAWeaponSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Weapon")
	EAAAWeaponType WeaponType = EAAAWeaponType::OneHandedSword;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Weapon")
	FAAAWeaponVisualSettings Visuals;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Weapon")
	FAAAWeaponCombatSettings Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Weapon")
	FAAAWeaponAbilitySettings Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Weapon")
	FAAAWeaponAnimationSettings Animations;
};
