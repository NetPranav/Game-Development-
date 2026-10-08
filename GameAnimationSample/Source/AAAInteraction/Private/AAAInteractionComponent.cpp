// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAInteractionComponent.h"
#include "Net/UnrealNetwork.h"

UAAAInteractionComponent::UAAAInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAAAInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAAAInteractionComponent::TryInteract()
{
	// Trace logic framework stub
}
