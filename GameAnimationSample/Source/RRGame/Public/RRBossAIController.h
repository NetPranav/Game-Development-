// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RREnemyAIController.h"
#include "RRBoss.h"
#include "RRBossAIController.generated.h"

UCLASS()
class RRGAME_API ARRBossAIController : public ARREnemyAIController
{
	GENERATED_BODY()

public:
	ARRBossAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ThinkLoop() override;
	virtual void HandleMovement() override;
	virtual void HandleCombat() override;
	virtual void HandleStaggerAndReactions() override;
	virtual void TransitionTo(ERREnemyState NewState) override;
	virtual void Tick(float DeltaSeconds) override;

	/** True when the level has a built navmesh; without one the boss steers straight at the player. */
	bool HasNavMesh() const;

public:
	/** Dormant: no combat AI (before the duel / entrance walk). Optionally walks to a point and faces another. */
	void SetDormant(bool bInDormant) { bDormant = bInDormant; if (bInDormant) { StopMovement(); SteerDir = FVector::ZeroVector; } }
	bool IsDormant() const { return bDormant; }
	void WalkTo(const FVector& Location, const FVector& FaceLocation) { WalkTarget = Location; FaceTarget = FaceLocation; bHasWalkTarget = true; }
	bool IsWalking() const { return bHasWalkTarget; }

private:
	/** Movement input re-applied every frame (ThinkLoop only runs every 0.1 s). */
	FVector SteerDir = FVector::ZeroVector;
	bool bDormant = false;
	bool bHasWalkTarget = false;
	FVector WalkTarget = FVector::ZeroVector;
	FVector FaceTarget = FVector::ZeroVector;
	void ThinkDormant();
	float SteerScale = 1.0f;

	int32 PressureAttacksRemaining = 0;
	bool bIsBackToBackPerilous = false;
	float BossAttackTimer = 0.0f;
	
	TWeakObjectPtr<ARRBoss> BossEnemy;
};
