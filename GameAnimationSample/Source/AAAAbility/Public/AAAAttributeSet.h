// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AAAAttributeSet.generated.h"

// Boilerplate macros to safely expose attribute accessors
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base Attribute Set for the AAA Framework.
 * Contains core stats used across all characters (Player, NPC, Enemies).
 */
UCLASS()
class AAAABILITY_API UAAAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAAAAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/* -------------------------------------------------------------------
	 * Attributes
	 * ------------------------------------------------------------------- */

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AAA|Attributes|Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Health)
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "AAA|Attributes|Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, MaxHealth)
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "AAA|Attributes|Vital")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Stamina)
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "AAA|Attributes|Vital")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, MaxStamina)
	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Poise, Category = "AAA|Attributes|Combat")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Poise)
	UFUNCTION()
	virtual void OnRep_Poise(const FGameplayAttributeData& OldPoise);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoise, Category = "AAA|Attributes|Combat")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, MaxPoise)
	UFUNCTION()
	virtual void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "AAA|Attributes|Combat")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Armor)
	UFUNCTION()
	virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	/** Meta attribute for calculating incoming damage. Not replicated. */
	UPROPERTY(BlueprintReadOnly, Category = "AAA|Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Damage)

	/** Meta attribute for incoming poise damage. Not replicated. */
	UPROPERTY(BlueprintReadOnly, Category = "AAA|Attributes|Meta")
	FGameplayAttributeData IncomingPoiseDamage;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, IncomingPoiseDamage)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "AAA|Attributes|Movement")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, MovementSpeed)
	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Experience, Category = "AAA|Attributes|Progression")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Experience)
	UFUNCTION()
	virtual void OnRep_Experience(const FGameplayAttributeData& OldExperience);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "AAA|Attributes|Progression")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UAAAAttributeSet, Level)
	UFUNCTION()
	virtual void OnRep_Level(const FGameplayAttributeData& OldLevel);
};
