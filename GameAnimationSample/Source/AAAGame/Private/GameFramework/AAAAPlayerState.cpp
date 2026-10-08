// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "GameFramework/AAAAPlayerState.h"
#include "AAAAbilitySystemComponent.h"
#include "AAAAttributeSet.h"
#include "Net/UnrealNetwork.h"

AAAAAPlayerState::AAAAAPlayerState()
{
	// Create the Ability System Component — owned by PlayerState, survives Pawn death
	AbilitySystemComponent = CreateDefaultSubobject<UAAAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the core Attribute Set — registered automatically to the ASC
	AttributeSet = CreateDefaultSubobject<UAAAAttributeSet>(TEXT("AttributeSet"));

	// PlayerState must replicate for all clients to see it
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AAAAAPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAAAAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Future: DOREPLIFETIME(AAAAAPlayerState, PlayerLevel);
	// Future: DOREPLIFETIME(AAAAAPlayerState, PlayerExperience);
	// Future: DOREPLIFETIME(AAAAAPlayerState, Gold);
}

void AAAAAPlayerState::GrantStartupAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& InAbilities,
	const TArray<TSubclassOf<UGameplayEffect>>& InEffects,
	const FGameplayTagContainer& InStartingTags
)
{
	if (bAbilitiesGranted) return;
	if (!HasAuthority()) return;
	if (!AbilitySystemComponent) return;

	// Grant Startup Abilities and Effects
	AbilitySystemComponent->GrantStartupData(InAbilities, InEffects);

	// Grant Starting Tags
	for (const FGameplayTag& Tag : InStartingTags)
	{
		AbilitySystemComponent->AddLooseGameplayTag(Tag);
	}

	bAbilitiesGranted = true;
}
