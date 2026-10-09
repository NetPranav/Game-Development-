// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AAACombatComponent.h"
#include "AAAGameplayAbility_Melee.generated.h"

class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class AAAABILITY_API UAAAGameplayAbility_Melee : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UAAAGameplayAbility_Melee();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void PlayAttack(EAAAAttackType Type);
	float GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo, EAAAAttackType Type) const;
	void ApplyStaminaCost(float Cost);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboInputEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboWindowOpened();

	UFUNCTION()
	void OnComboWindowClosed();

	UFUNCTION()
	void OnTraceWindowOpened();

	UFUNCTION()
	void OnTraceWindowClosed();

	void CheckComboTransition();

	UFUNCTION()
	void OnCancelTagChanged(const FGameplayTag Tag, int32 NewCount);

	void ClearAbilityTimers();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Combat")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Combat")
	FGameplayTag HitGameplayCueTag;

	UPROPERTY(Transient)
	int32 CurrentComboIndex;

	UPROPERTY(Transient)
	float LastAttackTime;

	UPROPERTY(Transient)
	bool bComboWindowOpen;

	UPROPERTY(Transient)
	bool bComboNextAttackTriggered;

	/** The attack montage playing right now: end/interrupt events from an older montage (the one this attack just
	 *  replaced) must not end the ability or chain the next attack. */
	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> CurrentAttackMontage;
	bool IsStaleMontageEvent() const;

	/** True while the current light attack is the last step of the combo (the finisher). */
	bool bCurrentIsFinisher = false;

	/** Players only: turns toward the best target (or movement input) and lunges forward so swings connect. */
	void ApplyAttackAssist(EAAAAttackType Type);

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> HitEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboInputTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboWindowOpenTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboWindowCloseTask;

	FTimerHandle FallbackComboOpenTimer;
	FTimerHandle FallbackComboCloseTimer;
	FTimerHandle FallbackTraceOpenTimer;
	FTimerHandle FallbackTraceCloseTimer;
	FTimerHandle ProceduralAttackTimer;

	FDelegateHandle StaggerDelegateHandle;
	FDelegateHandle DeadDelegateHandle;
};
