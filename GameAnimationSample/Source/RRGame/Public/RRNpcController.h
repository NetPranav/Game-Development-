// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RRNpcController.generated.h"

/**
 * Ambient NPC (Game Animation Sample character with a retargeted look): strolls around its spawn point, stops and
 * watches the duel. Uses the navmesh when there is one, otherwise steers directly (motion matching animates both).
 */
UCLASS()
class RRGAME_API ARRNpcController : public AAIController
{
	GENERATED_BODY()

public:
	ARRNpcController();

	virtual void Tick(float DeltaSeconds) override;

	/** How far from home the NPC may wander (cm). */
	UPROPERTY(EditAnywhere, Category = "RR|NPC")
	float WanderRadius = 500.0f;

	/** Movement input scale while strolling (GASP picks walk vs run from speed). */
	UPROPERTY(EditAnywhere, Category = "RR|NPC")
	float StrollInputScale = 0.45f;

	/** Spectator: stands at the platform edge, cheers (dance) during the duel, chats otherwise. */
	void MakeSpectator(class ARRArena* InArena);
	/** Wanderers keep out of the platform. */
	void SetArena(class ARRArena* InArena) { Arena = InArena; }

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	void PickNextGoal();
	void TickSpectator(APawn* P, double Now);
	void PlayMontage(APawn* P, const TCHAR* Path);
	TWeakObjectPtr<class ARRArena> Arena;
	bool bSpectator = false;
	double NextEmoteTime = 0.0;

	FVector Home = FVector::ZeroVector;
	FVector Goal = FVector::ZeroVector;
	bool bMoving = false;
	bool bDirectSteer = false;
	double NextDecisionTime = 0.0;
	double MoveStartTime = 0.0;
};
