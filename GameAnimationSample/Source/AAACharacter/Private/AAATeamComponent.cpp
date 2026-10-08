// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAATeamComponent.h"
#include "Net/UnrealNetwork.h"

UAAATeamComponent::UAAATeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	TeamID = 255; // Default generic unassigned team
}

void UAAATeamComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAAATeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAAATeamComponent, TeamID);
}

void UAAATeamComponent::OnRep_TeamID(uint8 OldTeamID)
{
	OnTeamChanged.Broadcast(TeamID);
}

void UAAATeamComponent::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TeamID = NewTeamID.GetId();
		OnTeamChanged.Broadcast(TeamID);
	}
}

FGenericTeamId UAAATeamComponent::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

ETeamAttitude::Type UAAATeamComponent::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other))
	{
		if (OtherTeamAgent->GetGenericTeamId() == GetGenericTeamId())
		{
			return ETeamAttitude::Friendly;
		}
		else
		{
			return ETeamAttitude::Hostile;
		}
	}
	return ETeamAttitude::Neutral;
}
