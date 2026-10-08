// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RREnemyAIController.generated.h"

class ARREnemy;

UENUM(BlueprintType)
enum class ERREnemyState : uint8
{
	Idle,
	Chase,
	Circle,
	Attack,
	Recover,
	Staggered,
	Dead
};

UCLASS()
class RRGAME_API ARREnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARREnemyAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintPure, Category = "RR|AI")
	ERREnemyState GetState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "RR|AI")
	AActor* GetTarget() const { return TargetActor.Get(); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void ThinkLoop();
	virtual void AcquireTarget();
	virtual void HandleMovement();
	virtual void HandleCombat();
	virtual void HandleStaggerAndReactions();
	virtual void TransitionTo(ERREnemyState NewState);
	virtual void ReleaseToken();

	UPROPERTY(Transient)
	ERREnemyState CurrentState = ERREnemyState::Idle;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> TargetActor;

	FTimerHandle ThinkTimerHandle;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARREnemy> ControlledEnemy;

	float CurrentCircleDirection = 1.0f;
	float LastCircleChangeTime = 0.0f;
	float RecoveryEndTime = 0.0f;
	float AttackStartTime = 0.0f;
	
	bool bHasAttackToken = false;
	float LastSidestepTime = 0.0f;

private:
	/** Next time (world seconds) the optional diagnostics line may be logged (rr.ai.debug >= 2). */
	float NextDiagTime = 0.f;
};
