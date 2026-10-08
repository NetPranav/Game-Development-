// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "AAATeamComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAAAOnTeamChangedSignature, uint8, NewTeamID);

/**
 * Component that implements the IAAATeamAgent interface logic.
 * Manages faction and team alignment for AI and Players.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAACHARACTER_API UAAATeamComponent : public UActorComponent, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:	
	UAAATeamComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	uint8 TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID(uint8 OldTeamID);

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface Interface
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Team")
	FAAAOnTeamChangedSignature OnTeamChanged;
};
