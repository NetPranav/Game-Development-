// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAKnockbackSubsystem.h"
#include "AAACombatEvents.h"
#include "AAACombatSettings.h"
#include "AAAMoverUtils.h"
#include "AAAGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/HitResult.h"

void UAAAKnockbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client) // authority only (standalone, listen server, dedicated server)
	{
		HitConfirmedHandle = FAAACombatEvents::OnHitConfirmed().AddUObject(this, &UAAAKnockbackSubsystem::OnHitConfirmed);
		StaggeredHandle = FAAACombatEvents::OnStaggered().AddUObject(this, &UAAAKnockbackSubsystem::OnStaggered);
	}
}

void UAAAKnockbackSubsystem::Deinitialize()
{
	if (HitConfirmedHandle.IsValid())
	{
		FAAACombatEvents::OnHitConfirmed().Remove(HitConfirmedHandle);
	}
	if (StaggeredHandle.IsValid())
	{
		FAAACombatEvents::OnStaggered().Remove(StaggeredHandle);
	}

	Super::Deinitialize();
}

void UAAAKnockbackSubsystem::OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	if (!Victim || !Attacker)
	{
		return;
	}

	LastAttackerMap.Add(Victim, Attacker);

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Victim);
	if (ASC && ASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
	{
		return;
	}

	const UAAACombatSettings* Settings = GetDefault<UAAACombatSettings>();
	if (!Settings) return;

	float Speed = bHeavy ? Settings->KnockbackHeavy : Settings->KnockbackLight;
	
	FVector AwayDir = Victim->GetActorLocation() - Attacker->GetActorLocation();
	AwayDir.Z = 0.0f;
	if (AwayDir.IsNearlyZero())
	{
		AwayDir = -Victim->GetActorForwardVector();
		AwayDir.Z = 0.0f;
	}
	AwayDir.Normalize();

	float Distance = bHeavy ? 220.0f : 115.0f; // Middle of 180-260 and 80-150
	if (ASC && ASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Blocking))
	{
		Distance = 55.0f; // Middle of 40-70
	}

	UAAAMoverUtils::ApplyVelocityBurst(Victim, AwayDir * (Distance / 0.2f), 0.2f);
}

void UAAAKnockbackSubsystem::OnStaggered(AActor* Victim)
{
	if (!Victim)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Victim);
	if (ASC && ASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
	{
		return;
	}

	const UAAACombatSettings* Settings = GetDefault<UAAACombatSettings>();
	if (!Settings) return;

	FVector AwayDir = -Victim->GetActorForwardVector();
	if (TWeakObjectPtr<AActor>* AttackerPtr = LastAttackerMap.Find(Victim))
	{
		if (AActor* Attacker = AttackerPtr->Get())
		{
			AwayDir = Victim->GetActorLocation() - Attacker->GetActorLocation();
		}
	}

	AwayDir.Z = 0.0f;
	if (AwayDir.IsNearlyZero())
	{
		AwayDir = -Victim->GetActorForwardVector();
		AwayDir.Z = 0.0f;
	}
	AwayDir.Normalize();

	UAAAMoverUtils::ApplyVelocityBurst(Victim, AwayDir * (Settings->KnockbackStagger / 0.2f), 0.2f);
}
