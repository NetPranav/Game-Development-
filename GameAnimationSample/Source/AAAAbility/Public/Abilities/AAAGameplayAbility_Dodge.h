// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AAAGameplayAbility_Dodge.generated.h"

class UAnimMontage;

/**
 * Sekiro-style Directional Step-Dodge Gameplay Ability.
 * 1. Computes directional dodge vector based on player movement input.
 * 2. Grants temporary invulnerability i-frames (State.Combat.Invulnerable).
 * 3. Applies a rapid grounded step impulse.
 * 4. Deducts Stamina cost.
 */
UCLASS()
class AAAABILITY_API UAAAGameplayAbility_Dodge : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAAAGameplayAbility_Dodge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnDodgeFinished();

	UFUNCTION()
	void OnIFrameStart();

	UFUNCTION()
	void OnIFrameEnd();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	TObjectPtr<UAnimMontage> DodgeForwardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	TObjectPtr<UAnimMontage> DodgeBackwardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	TObjectPtr<UAnimMontage> DodgeLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	TObjectPtr<UAnimMontage> DodgeRightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	float DodgeImpulseStrength = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	float DodgeDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	float StaminaCost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	float IFrameStart = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Dodge")
	float IFrameEnd = 0.30f;

private:
	FTimerHandle DodgeTimerHandle;
	FTimerHandle IFrameStartTimer;
	FTimerHandle IFrameEndTimer;
	bool bIsEnding = false;
};
