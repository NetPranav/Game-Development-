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

private:
	int32 PressureAttacksRemaining = 0;
	bool bIsBackToBackPerilous = false;
	float BossAttackTimer = 0.0f;
	
	TWeakObjectPtr<ARRBoss> BossEnemy;
};
