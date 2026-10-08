// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AAAAttributeComponent.generated.h"

class UAAAAbilitySystemComponent;
class UAAAAttributeSet;
class UGameplayEffect;

// Health, MaxHealth, PreviousValue
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAAAOnHealthChangedSignature, float, Health, float, MaxHealth, float, PreviousHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAAAOnDeathSignature);

/**
 * Component responsible for interfacing with the Ability System Component's attributes.
 * Broadcasts changes to vital stats (Health, Stamina) to UI and other systems.
 *
 * *** ARCHITECTURAL NOTE ***
 * TakeDamage/Heal use pre-authored GameplayEffect classes with SetByCallerMagnitude
 * instead of dynamically allocating UGameplayEffect objects. This eliminates GC pressure
 * during combat — zero heap allocations per hit.
 *
 * Required Blueprint Setup:
 * - Assign DirectDamageEffectClass to a GE with SetByCaller magnitude (tag: Data.Damage)
 * - Assign DirectHealEffectClass to a GE with SetByCaller magnitude (tag: Data.Healing)
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAACHARACTER_API UAAAAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAAAttributeComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAAAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<const UAAAAttributeSet> AttributeSet;

public:
	/** Hook up the ASC to this component */
	void InitializeWithAbilitySystem(UAAAAbilitySystemComponent* InASC);

	// -------------------------------------------------------------------
	// Pre-Authored Gameplay Effect Classes (Zero Allocation)
	//
	// These must be set in the Blueprint defaults to a GE asset that uses
	// SetByCallerMagnitude with the appropriate Data tag.
	// -------------------------------------------------------------------

	/** GE class for direct damage. Must use SetByCaller with Data.Damage tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Attributes|Effects")
	TSubclassOf<UGameplayEffect> DirectDamageEffectClass;

	/** GE class for direct healing. Must use SetByCaller with Data.Healing tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Attributes|Effects")
	TSubclassOf<UGameplayEffect> DirectHealEffectClass;

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------
	
	UPROPERTY(BlueprintAssignable, Category = "AAA|Attributes")
	FAAAOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Attributes")
	FAAAOnDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Attributes")
	FAAAOnDeathSignature OnRevived;

	// -------------------------------------------------------------------
	// Queries
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	bool IsDead() const;

	// -------------------------------------------------------------------
	// Gameplay Interface (Zero-Allocation via SetByCallerMagnitude)
	// -------------------------------------------------------------------

	/** Apply damage via pre-authored GE. Zero heap allocation. */
	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	virtual void TakeDamage(float DamageAmount);

	/** Apply healing via pre-authored GE. Zero heap allocation. */
	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	virtual void Heal(float HealAmount);

	/** Instant kill — directly sets Health to 0. Bypasses damage resistance. */
	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	virtual void Kill();

	/** Revive — restores full health and swaps state tags. */
	UFUNCTION(BlueprintCallable, Category = "AAA|Attributes")
	virtual void Revive();

protected:
	// Internal ASC Callbacks
	virtual void HealthChanged(const struct FOnAttributeChangeData& Data);
	virtual void StaminaChanged(const struct FOnAttributeChangeData& Data);

	bool bIsDead;
};
