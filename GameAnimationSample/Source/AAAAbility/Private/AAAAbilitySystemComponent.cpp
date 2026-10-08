// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

UAAAAbilitySystemComponent::UAAAAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bCharacterAbilitiesGiven = false;
}

void UAAAAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAAAAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

void UAAAAbilitySystemComponent::OnAbilitySystemInitialized()
{
	// Base initialization logic if needed
}

void UAAAAbilitySystemComponent::GrantStartupData(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities, const TArray<TSubclassOf<UGameplayEffect>>& InEffects)
{
	if (bCharacterAbilitiesGiven)
	{
		return;
	}

	if (!GetOwnerActor()->HasAuthority())
	{
		return;
	}

	// Grant Abilities
	for (const TSubclassOf<UGameplayAbility>& StartupAbility : InAbilities)
	{
		if (StartupAbility)
		{
			// GiveAbility adds the ability and creates a handle
			FGameplayAbilitySpec Spec(StartupAbility, 1, INDEX_NONE, this->GetAvatarActor());
			GiveAbility(Spec);
		}
	}

	// Apply Startup Effects (e.g., initial stats, base health regen)
	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddInstigator(GetOwnerActor(), GetAvatarActor());

	for (const TSubclassOf<UGameplayEffect>& StartupEffect : InEffects)
	{
		if (StartupEffect)
		{
			FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(StartupEffect, 1, EffectContext);
			if (NewHandle.IsValid())
			{
				ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}

	bCharacterAbilitiesGiven = true;
}
