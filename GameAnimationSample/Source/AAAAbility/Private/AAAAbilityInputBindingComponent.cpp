// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAbilityInputBindingComponent.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"

UAAAAbilityInputBindingComponent::UAAAAbilityInputBindingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAAAAbilityInputBindingComponent::SetAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	AbilitySystemComponent = InASC;
}

void UAAAAbilityInputBindingComponent::SetupBindings(UEnhancedInputComponent* InputComponent, const TArray<FAAAAbilityInputBinding>& Bindings)
{
	if (!InputComponent) return;

	ActiveBindings = Bindings;

	for (const FAAAAbilityInputBinding& Binding : ActiveBindings)
	{
		if (!Binding.InputAction || !Binding.InputTag.IsValid())
		{
			continue;
		}

		// Bind Started → activate the ability matching this tag
		InputComponent->BindAction(Binding.InputAction, ETriggerEvent::Started, this,
			&UAAAAbilityInputBindingComponent::OnAbilityInputStarted, Binding.InputTag);

		// Bind Completed → optionally cancel held abilities
		InputComponent->BindAction(Binding.InputAction, ETriggerEvent::Completed, this,
			&UAAAAbilityInputBindingComponent::OnAbilityInputCompleted, Binding.InputTag, Binding.bCancelOnRelease);
	}
}

void UAAAAbilityInputBindingComponent::ClearBindings(UEnhancedInputComponent* InputComponent)
{
	// Enhanced Input handles cleanup when the component is destroyed
	ActiveBindings.Empty();
}

void UAAAAbilityInputBindingComponent::OnAbilityInputStarted(const FGameplayTag InputTag)
{
	if (!AbilitySystemComponent.IsValid() || !InputTag.IsValid()) return;

	// Build a tag container with just this one input tag
	FGameplayTagContainer TagFilter;
	TagFilter.AddTag(InputTag);

	// Try to activate any ability whose AbilityTags match this InputTag
	// This is the core of the Lyra pattern: abilities declare their InputTag
	// in their AbilityTags, and this component fires the matching tag
	AbilitySystemComponent->TryActivateAbilitiesByTag(TagFilter);
}

void UAAAAbilityInputBindingComponent::OnAbilityInputCompleted(const FGameplayTag InputTag, bool bCancelOnRelease)
{
	if (!AbilitySystemComponent.IsValid() || !InputTag.IsValid()) return;

	if (bCancelOnRelease)
	{
		// Cancel any active ability that has this input tag
		FGameplayTagContainer TagFilter;
		TagFilter.AddTag(InputTag);
		AbilitySystemComponent->CancelAbilities(&TagFilter);
	}
}
