// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAttributeComponent.h"
#include "AAAAbilitySystemComponent.h"
#include "AAAAttributeSet.h"
#include "AAAGameplayTags.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UAAAAttributeComponent::UAAAAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsDead = false;
}

void UAAAAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAAAAttributeComponent::InitializeWithAbilitySystem(UAAAAbilitySystemComponent* InASC)
{
	if (!InASC) return;

	AbilitySystemComponent = InASC;
	AttributeSet = InASC->GetSet<UAAAAttributeSet>();

	if (AttributeSet.IsValid())
	{
		// Bind to ASC Attribute Change Delegates
		InASC->GetGameplayAttributeValueChangeDelegate(UAAAAttributeSet::GetHealthAttribute()).AddUObject(this, &UAAAAttributeComponent::HealthChanged);
		InASC->GetGameplayAttributeValueChangeDelegate(UAAAAttributeSet::GetStaminaAttribute()).AddUObject(this, &UAAAAttributeComponent::StaminaChanged);
	}
}

float UAAAAttributeComponent::GetHealth() const
{
	return AttributeSet.IsValid() ? AttributeSet->GetHealth() : 0.0f;
}

float UAAAAttributeComponent::GetMaxHealth() const
{
	return AttributeSet.IsValid() ? AttributeSet->GetMaxHealth() : 0.0f;
}

float UAAAAttributeComponent::GetStamina() const
{
	return AttributeSet.IsValid() ? AttributeSet->GetStamina() : 0.0f;
}

float UAAAAttributeComponent::GetMaxStamina() const
{
	return AttributeSet.IsValid() ? AttributeSet->GetMaxStamina() : 0.0f;
}

bool UAAAAttributeComponent::IsDead() const
{
	return bIsDead;
}

void UAAAAttributeComponent::TakeDamage(float DamageAmount)
{
	if (IsDead() || !AbilitySystemComponent.IsValid() || DamageAmount <= 0.0f) return;
	if (AbilitySystemComponent->HasMatchingGameplayTag(FAAAGameplayTags::Get().Status_Invincible)) return;

	// *** ZERO ALLOCATION PATTERN ***
	// Uses a pre-authored GameplayEffect Blueprint with SetByCallerMagnitude.
	// No NewObject<UGameplayEffect> — no GC pressure during combat.
	if (!DirectDamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAAAAttributeComponent::TakeDamage - DirectDamageEffectClass is not set! Assign a GE_DirectDamage asset in Blueprint defaults."));
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(GetOwner(), GetOwner());

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DirectDamageEffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		// Set the damage magnitude via the Data.Damage tag — the GE reads this at execution time
		SpecHandle.Data->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_Damage, -DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UAAAAttributeComponent::Heal(float HealAmount)
{
	if (IsDead() || !AbilitySystemComponent.IsValid() || HealAmount <= 0.0f) return;

	// *** ZERO ALLOCATION PATTERN ***
	if (!DirectHealEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAAAAttributeComponent::Heal - DirectHealEffectClass is not set! Assign a GE_DirectHeal asset in Blueprint defaults."));
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(GetOwner(), GetOwner());

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DirectHealEffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_Healing, HealAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UAAAAttributeComponent::Kill()
{
	if (!AbilitySystemComponent.IsValid()) return;

	// *** DIRECT ATTRIBUTE SET — NOT A HACK ***
	// Directly sets Health to 0, bypassing damage resistance, invincibility, and armor.
	// The HealthChanged callback handles death logic (state tags, delegate broadcast).
	AbilitySystemComponent->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), 0.0f);
}

void UAAAAttributeComponent::Revive()
{
	if (!IsDead() || !AbilitySystemComponent.IsValid()) return;

	bIsDead = false;

	// Restore full health via the proper GE pipeline
	Heal(GetMaxHealth());

	AbilitySystemComponent->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Dead);
	AbilitySystemComponent->AddLooseGameplayTag(FAAAGameplayTags::Get().State_Alive);
	
	OnRevived.Broadcast();
}

void UAAAAttributeComponent::HealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue, GetMaxHealth(), Data.OldValue);

	// Check Death
	// Segmented bosses sit at 0 health between segments (deathblow pending); only the boss itself ends the fight.
	const bool bSegmentedBoss = AbilitySystemComponent.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Boss_Segmented);
	if (Data.NewValue <= 0.0f && !bIsDead && !bSegmentedBoss)
	{
		bIsDead = true;

		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Alive);
			AbilitySystemComponent->AddLooseGameplayTag(FAAAGameplayTags::Get().State_Dead);
		}

		OnDeath.Broadcast();
	}
}

void UAAAAttributeComponent::StaminaChanged(const FOnAttributeChangeData& Data)
{
	// Future: Broadcast stamina changes to a stamina delegate for UI
}
