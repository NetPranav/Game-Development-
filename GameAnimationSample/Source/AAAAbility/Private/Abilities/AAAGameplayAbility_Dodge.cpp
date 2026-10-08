// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Abilities/AAAGameplayAbility_Dodge.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AAAGameplayTags.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "AAADebugLogger.h"
#include "AAAMoverUtils.h"
#include "Effects/AAAGE_StaminaCost.h"
#include "AAACombatSettings.h"
#include "AAAAttributeSet.h"

UAAAGameplayAbility_Dodge::UAAAGameplayAbility_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	AbilityTags.AddTag(FAAAGameplayTags::Get().InputTag_Dodge);
}

bool UAAAGameplayAbility_Dodge::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return false;

	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
	if (ASC->HasMatchingGameplayTag(Tags.State_Combat_Dead) || ASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered))
	{
		return false;
	}

	float ActualCost = StaminaCost > 0.0f ? StaminaCost : UAAACombatSettings::Get()->DodgeStaminaCost;
	FGameplayAttribute StaminaAttr = ASC->GetAttributeSet(UAAAAttributeSet::StaticClass()) ? FGameplayAttribute(FindFProperty<FProperty>(ASC->GetAttributeSet(UAAAAttributeSet::StaticClass())->GetClass(), FName("Stamina"))) : FGameplayAttribute();
	if (!StaminaAttr.IsValid()) 
	{
		// Try dynamic search
		for (const UAttributeSet* Set : ASC->GetSpawnedAttributes())
		{
			if (Set)
			{
				if (FProperty* Prop = FindFProperty<FProperty>(Set->GetClass(), FName("Stamina")))
				{
					StaminaAttr = FGameplayAttribute(Prop);
					break;
				}
			}
		}
	}

	if (StaminaAttr.IsValid())
	{
		bool bFound = false;
		float Stamina = ASC->GetGameplayAttributeValue(StaminaAttr, bFound);
		if (bFound && Stamina < ActualCost)
		{
			return false;
		}
	}

	return true;
}

void UAAAGameplayAbility_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsEnding = false;

	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!Pawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// Cancel active melee attacks
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(FAAAGameplayTags::Get().State_Combat_Attacking);
		ASC->CancelAbilities(&CancelTags);

		// Apply stamina cost
		float ActualCost = StaminaCost > 0.0f ? StaminaCost : UAAACombatSettings::Get()->DodgeStaminaCost;
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpec* Spec = new FGameplayEffectSpec(UAAAGE_StaminaCost::StaticClass()->GetDefaultObject<UGameplayEffect>(), Context, 1.0f);
		Spec->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_StaminaCost, -ActualCost);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	}

	FVector MoveIntent = UAAAMoverUtils::GetMoveIntent(Pawn);
	FVector DodgeDirection = Pawn->GetActorForwardVector();
	UAnimMontage* MontageToPlay = DodgeForwardMontage;

	FString DirectionStr = TEXT("FORWARD");
	if (!MoveIntent.IsNearlyZero())
	{
		MoveIntent.Normalize();
		const float ForwardDot = FVector::DotProduct(Pawn->GetActorForwardVector(), MoveIntent);
		const float RightDot = FVector::DotProduct(Pawn->GetActorRightVector(), MoveIntent);

		if (ForwardDot > 0.5f)
		{
			DodgeDirection = Pawn->GetActorForwardVector();
			MontageToPlay = DodgeForwardMontage;
			DirectionStr = TEXT("FORWARD");
		}
		else if (ForwardDot < -0.5f)
		{
			DodgeDirection = -Pawn->GetActorForwardVector();
			MontageToPlay = DodgeBackwardMontage;
			DirectionStr = TEXT("BACKWARD");
		}
		else if (RightDot > 0.0f)
		{
			DodgeDirection = Pawn->GetActorRightVector();
			MontageToPlay = DodgeRightMontage;
			DirectionStr = TEXT("RIGHT");
		}
		else
		{
			DodgeDirection = -Pawn->GetActorRightVector();
			MontageToPlay = DodgeLeftMontage;
			DirectionStr = TEXT("LEFT");
		}
	}
	else
	{
		DodgeDirection = -Pawn->GetActorForwardVector();
		MontageToPlay = DodgeBackwardMontage;
		DirectionStr = TEXT("BACKSTEP");
	}

	FAAADebugLogger::LogAction(TEXT("DODGE"), FString::Printf(TEXT("Dodge: %s"), *DirectionStr), FColor(0, 255, 200), 2.5f);

	UAAAMoverUtils::ApplyVelocityBurst(Pawn, DodgeDirection * DodgeImpulseStrength, DodgeDuration);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(IFrameStartTimer, this, &UAAAGameplayAbility_Dodge::OnIFrameStart, IFrameStart, false);
		World->GetTimerManager().SetTimer(IFrameEndTimer, this, &UAAAGameplayAbility_Dodge::OnIFrameEnd, IFrameEnd, false);
	}

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (Character && MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.25f);

		PlayMontageTask->OnCompleted.AddDynamic(this, &UAAAGameplayAbility_Dodge::OnDodgeFinished);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UAAAGameplayAbility_Dodge::OnDodgeFinished);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UAAAGameplayAbility_Dodge::OnDodgeFinished);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UAAAGameplayAbility_Dodge::OnDodgeFinished);
		PlayMontageTask->ReadyForActivation();
	}
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DodgeTimerHandle, this, &UAAAGameplayAbility_Dodge::OnDodgeFinished, DodgeDuration, false);
	}
}

void UAAAGameplayAbility_Dodge::OnIFrameStart()
{
	if (bIsEnding) return;
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Invulnerable);
	}
}

void UAAAGameplayAbility_Dodge::OnIFrameEnd()
{
	if (bIsEnding) return;
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Invulnerable);
	}
}

void UAAAGameplayAbility_Dodge::OnDodgeFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAAAGameplayAbility_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Invulnerable);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeTimerHandle);
		World->GetTimerManager().ClearTimer(IFrameStartTimer);
		World->GetTimerManager().ClearTimer(IFrameEndTimer);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
