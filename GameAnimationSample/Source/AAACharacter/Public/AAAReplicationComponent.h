// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AAAReplicationComponent.generated.h"

/**
 * Handles generic replication logic, state synchronization, and rollback 
 * helpers for the AAA Framework outside of what Mover natively provides.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAACHARACTER_API UAAAReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAAReplicationComponent();

protected:
	virtual void BeginPlay() override;

public:
	// Example method for forcing a state sync
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSync();
};
