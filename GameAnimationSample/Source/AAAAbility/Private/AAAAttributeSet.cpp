// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AAADebugLogger.h"
#include "AAAGameplayTags.h"
#include "AAACombatSettings.h"
#include "AAACombatComponent.h"

UAAAAttributeSet::UAAAAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitPoise(100.0f);
	InitMaxPoise(100.0f);
	InitMovementSpeed(500.0f);
	InitExperience(0.0f);
	InitLevel(1.0f);
	InitArmor(0.0f);
	InitDamage(0.0f);
	InitIncomingPoiseDamage(0.0f);
}

void UAAAAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Experience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAAAAttributeSet, Armor, COND_None, REPNOTIFY_Always);
}

void UAAAAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp attributes as they change
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
	}
}

namespace
{
	/** The combat component lives on the avatar pawn, not on the attribute set's owner (the PlayerState for players). */
	UAAACombatComponent* FindCombatComponent(const FGameplayEffectModCallbackData& Data)
	{
		const AActor* Avatar = Data.Target.GetAvatarActor();
		return Avatar ? Avatar->FindComponentByClass<UAAACombatComponent>() : nullptr;
	}
}

void UAAAAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Ensure attributes stay within bounds after a Gameplay Effect executes
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0.0f)
		{
			UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
			if (!TargetASC)
			{
				return;
			}
			AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator();
			const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();

			if (TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Dead) || TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Invulnerable))
			{
				return;
			}

			if (TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Parrying))
			{
				if (UAbilitySystemComponent* SourceASC = SourceActor ? SourceActor->FindComponentByClass<UAbilitySystemComponent>() : nullptr)
				{
					FGameplayEventData Payload;
					Payload.EventTag = Tags.GameplayEvent_Combat_Parried;
					Payload.Target = GetOwningActor();
					SourceASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Parried, &Payload);
				}
				FGameplayCueParameters CueParams;
				TargetASC->ExecuteGameplayCue(Tags.GameplayCue_Combat_Hit_Parried, CueParams);
				return;
			}

			float FinalDamage = LocalDamageDone * 100.0f / (100.0f + GetArmor());

			if (UAAACombatComponent* TargetCombatComp = FindCombatComponent(Data))
			{
				FinalDamage *= (1.0f - TargetCombatComp->GetDamageReduction());
			}

			if (TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Blocking))
			{
				const UAAACombatSettings* CombatSettings = UAAACombatSettings::Get();
				FinalDamage *= (1.0f - CombatSettings->BlockDamageReduction);
				float StaminaCost = LocalDamageDone * CombatSettings->BlockStaminaPerDamage;
				
				float NewStamina = GetStamina() - StaminaCost;
				if (NewStamina <= 0.0f)
				{
					SetStamina(0.0f);
					TargetASC->RemoveLooseGameplayTag(Tags.State_Combat_Blocking);
					if (UAAACombatComponent* CombatComp = FindCombatComponent(Data))
					{
						CombatComp->ApplyTimedState(Tags.State_Combat_Staggered, CombatSettings->StaggerSeconds);
					}
				}
				else
				{
					SetStamina(NewStamina);
				}

				FGameplayEventData Payload;
				Payload.EventTag = Tags.GameplayEvent_Combat_Blocked;
				TargetASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Blocked, &Payload);
				
				FGameplayCueParameters CueParams;
				TargetASC->ExecuteGameplayCue(Tags.GameplayCue_Combat_Hit_Blocked, CueParams);
			}

			if (TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered))
			{
				FinalDamage *= UAAACombatSettings::Get()->PoiseBrokenDamageMultiplier;
			}

			const float NewHealth = FMath::Clamp(GetHealth() - FinalDamage, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			if (NewHealth <= 0.0f && !TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Dead))
			{
				FGameplayTag StateBossSegmented = FGameplayTag::RequestGameplayTag(FName("State.Boss.Segmented"), false);
				if (StateBossSegmented.IsValid() && TargetASC->HasMatchingGameplayTag(StateBossSegmented))
				{
					if (UAAACombatComponent* CombatComp = FindCombatComponent(Data))
					{
						float StaggerDuration = UAAACombatSettings::Get()->StaggerSeconds;
						if (AActor* TargetActor = GetOwningActor())
						{
							if (FFloatProperty* Prop = FindFProperty<FFloatProperty>(TargetActor->GetClass(), FName("BrokenSegmentStaggerSeconds")))
							{
								StaggerDuration = Prop->GetPropertyValue_InContainer(TargetActor);
							}
						}
						CombatComp->ApplyTimedState(Tags.State_Combat_Staggered, StaggerDuration);
					}
					
					FGameplayEventData Payload;
					Payload.EventTag = Tags.GameplayEvent_Combat_Staggered;
					TargetASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Staggered, &Payload);
				}
				else
				{
					if (TargetASC->GetOwnerRole() == ROLE_Authority)
					{
						TargetASC->AddLooseGameplayTag(Tags.State_Combat_Dead);
					}
					FGameplayEventData Payload;
					Payload.EventTag = Tags.GameplayEvent_Combat_Death;
					TargetASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Death, &Payload);
				}
			}

			FString TargetName = Data.Target.GetAvatarActor() ? Data.Target.GetAvatarActor()->GetName() : TEXT("Target");
			FAAADebugLogger::LogAction(TEXT("DAMAGE"), FString::Printf(TEXT("%s took %.1f Damage! Health: %.1f / %.1f"), *TargetName, FinalDamage, NewHealth, GetMaxHealth()), FColor::Red, 4.0f);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingPoiseDamageAttribute())
	{
		float LocalPoiseDamage = GetIncomingPoiseDamage();
		SetIncomingPoiseDamage(0.0f);

		if (LocalPoiseDamage > 0.0f)
		{
			UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
			const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
			if (!TargetASC
				|| TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Dead)
				|| TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Invulnerable)
				|| TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Parrying))
			{
				if (TargetASC && TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Parrying))
				{
					if (AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator())
					{
						if (UAbilitySystemComponent* SourceASC = SourceActor->FindComponentByClass<UAbilitySystemComponent>())
						{
							FGameplayAttribute InPoiseAttr = UAAAAttributeSet::GetIncomingPoiseDamageAttribute();
							if (InPoiseAttr.IsValid())
							{
								float CurrentIn = SourceASC->GetNumericAttribute(InPoiseAttr);
								SourceASC->SetNumericAttributeBase(InPoiseAttr, CurrentIn + LocalPoiseDamage * 1.5f);
							}
						}
					}
				}
				return; // Dodged / parried / dead targets take no poise damage.
			}

			if (TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Blocking))
			{
				LocalPoiseDamage *= 0.25f;
			}

			float NewPoise = GetPoise() - LocalPoiseDamage;
			if (NewPoise <= 0.0f && !TargetASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered))
			{
				if (UAAACombatComponent* CombatComp = FindCombatComponent(Data))
				{
					float StaggerDuration = UAAACombatSettings::Get()->StaggerSeconds;
					FGameplayTag BossTag = FGameplayTag::RequestGameplayTag(FName("Character.Boss"), false);
					if (BossTag.IsValid() && TargetASC->HasMatchingGameplayTag(BossTag))
					{
						if (AActor* TargetActor = GetOwningActor())
						{
							if (FFloatProperty* Prop = FindFProperty<FFloatProperty>(TargetActor->GetClass(), FName("StaggerSeconds")))
							{
								StaggerDuration = Prop->GetPropertyValue_InContainer(TargetActor);
							}
						}
					}
					CombatComp->ApplyTimedState(Tags.State_Combat_Staggered, StaggerDuration);
				}
				SetPoise(GetMaxPoise());

				FGameplayEventData Payload;
				Payload.EventTag = Tags.GameplayEvent_Combat_Staggered;
				TargetASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Staggered, &Payload);
			}
			else
			{
				SetPoise(FMath::Clamp(NewPoise, 0.0f, GetMaxPoise()));
			}
		}
	}
}

// -------------------------------------------------------------------
// OnRep Functions for Network Syncing
// -------------------------------------------------------------------

void UAAAAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Health, OldHealth);
}

void UAAAAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, MaxHealth, OldMaxHealth);
}

void UAAAAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Stamina, OldStamina);
}

void UAAAAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, MaxStamina, OldMaxStamina);
}

void UAAAAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UAAAAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Experience, OldExperience);
}

void UAAAAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Level, OldLevel);
}

void UAAAAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Armor, OldArmor);
}

void UAAAAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, Poise, OldPoise);
}

void UAAAAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAAAAttributeSet, MaxPoise, OldMaxPoise);
}
