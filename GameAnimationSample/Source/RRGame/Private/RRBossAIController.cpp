// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRBossAIController.h"
#include "RRBoss.h"
#include "AAACombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AAAGameplayTags.h"
#include "Kismet/GameplayStatics.h"

ARRBossAIController::ARRBossAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ARRBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BossEnemy = Cast<ARRBoss>(InPawn);
	
	if (BossEnemy.IsValid())
	{
		if (UCharacterMovementComponent* MoveComp = BossEnemy->GetCharacterMovement())
		{
			MoveComp->bUseControllerDesiredRotation = true;
			MoveComp->bOrientRotationToMovement = false;
		}
		BossEnemy->bUseControllerRotationYaw = false;
	}
}

void ARRBossAIController::TransitionTo(ERREnemyState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	
	if (CurrentState == ERREnemyState::Recover || CurrentState == ERREnemyState::Circle)
	{
		if (BossEnemy.IsValid())
		{
			float WaitMin = BossEnemy->GetPhase() == 2 ? 1.0f : 1.5f;
			float WaitMax = BossEnemy->GetPhase() == 2 ? 1.8f : 2.5f;
			RecoveryEndTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(WaitMin, WaitMax);
		}
	}
	else if (CurrentState == ERREnemyState::Attack)
	{
		if (PressureAttacksRemaining <= 0)
		{
			PressureAttacksRemaining = FMath::RandRange(3, 5);
		}
	}
}

void ARRBossAIController::HandleStaggerAndReactions()
{
	if (!BossEnemy.IsValid()) return;
	
	UAAACombatComponent* CombatComp = BossEnemy->CombatComponent;
	if (CombatComp && CombatComp->HasState(FAAAGameplayTags::Get().State_Combat_Staggered))
	{
		if (CurrentState != ERREnemyState::Staggered)
		{
			StopMovement();
			TransitionTo(ERREnemyState::Staggered);
		}
		return;
	}
	
	if (CurrentState == ERREnemyState::Staggered)
	{
		TransitionTo(ERREnemyState::Recover);
	}
	
	// Deflect/block logic when player attacks
	if (TargetActor.IsValid() && CombatComp && CurrentState != ERREnemyState::Attack && CurrentState != ERREnemyState::Staggered && CurrentState != ERREnemyState::Dead)
	{
		float DistSq = FVector::DistSquared(BossEnemy->GetActorLocation(), TargetActor->GetActorLocation());
		if (DistSq <= 350.0f * 350.0f)
		{
			UAAACombatComponent* TargetCombat = TargetActor->FindComponentByClass<UAAACombatComponent>();
			if (TargetCombat && TargetCombat->IsAttacking())
			{
				float TimeSecs = GetWorld()->GetTimeSeconds();
				if (TimeSecs - LastSidestepTime > 2.0f)
				{
					if (FMath::FRand() < BossEnemy->Tuning.BlockChanceWhenPlayerAttacking) // Prompt: 25% chance
					{
						LastSidestepTime = TimeSecs;
						CombatComp->SetBlocking(true);
						CombatComp->ApplyTimedState(FAAAGameplayTags::Get().State_Combat_Blocking, 0.8f);
					}
				}
			}
		}
	}
}

void ARRBossAIController::ThinkLoop()
{
	if (!BossEnemy.IsValid() || !BossEnemy->IsAlive())
	{
		if (CurrentState != ERREnemyState::Dead)
		{
			StopMovement();
			TransitionTo(ERREnemyState::Dead);
		}
		return;
	}
	
	AcquireTarget();
	HandleStaggerAndReactions();
	
	if (CurrentState == ERREnemyState::Dead || CurrentState == ERREnemyState::Staggered)
	{
		return;
	}
	
	if (!TargetActor.IsValid())
	{
		TransitionTo(ERREnemyState::Idle);
		return;
	}

	SetFocus(TargetActor.Get());
	
	FVector MyLoc = BossEnemy->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();
	float DistToTarget = FVector::Dist2D(MyLoc, TargetLoc);
	FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal2D();
	
	if (CurrentState == ERREnemyState::Idle)
	{
		TransitionTo(ERREnemyState::Chase);
	}
	
	if (CurrentState == ERREnemyState::Chase)
	{
		if (DistToTarget <= BossEnemy->Tuning.AttackRange * 1.5f)
		{
			if (PressureAttacksRemaining > 0)
				TransitionTo(ERREnemyState::Attack);
			else
				TransitionTo(ERREnemyState::Circle);
		}
		else
		{
			HandleMovement();
		}
	}
	
	if (CurrentState == ERREnemyState::Circle || CurrentState == ERREnemyState::Recover)
	{
		if (UCharacterMovementComponent* MoveComp = BossEnemy->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = BossEnemy->Tuning.ChaseSpeed;
		}

		if (GetWorld()->GetTimeSeconds() >= RecoveryEndTime)
		{
			PressureAttacksRemaining = FMath::RandRange(3, 5);
			TransitionTo(ERREnemyState::Attack);
		}
		else
		{
			float TimeSecs = GetWorld()->GetTimeSeconds();
			if (TimeSecs - LastCircleChangeTime > FMath::RandRange(1.5f, 3.0f))
			{
				CurrentCircleDirection *= -1.0f;
				LastCircleChangeTime = TimeSecs;
			}
			
			FVector Tangent = FVector::CrossProduct(FVector::UpVector, DirToTarget) * CurrentCircleDirection;
			BossEnemy->AddMovementInput(Tangent + (DistToTarget > BossEnemy->Tuning.PreferredCircleRange ? DirToTarget : -DirToTarget)*0.2f);
		}
	}
	
	if (CurrentState == ERREnemyState::Attack)
	{
		if (DistToTarget > BossEnemy->Tuning.AttackRange)
		{
			HandleMovement();
		}
		else
		{
			HandleCombat();
		}
	}
}

void ARRBossAIController::HandleMovement()
{
	if (!BossEnemy.IsValid() || !TargetActor.IsValid()) return;
	if (UCharacterMovementComponent* MoveComp = BossEnemy->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = BossEnemy->Tuning.ChaseSpeed;
	}
	MoveToActor(TargetActor.Get(), BossEnemy->Tuning.AttackRange * 0.8f, false, true, true);
}

void ARRBossAIController::HandleCombat()
{
	if (!BossEnemy.IsValid() || !TargetActor.IsValid()) return;
	
	if (GetWorld()->GetTimeSeconds() < BossAttackTimer)
	{
		return; // Wait for attack to finish
	}

	if (PressureAttacksRemaining <= 0 && !bIsBackToBackPerilous)
	{
		TransitionTo(ERREnemyState::Recover);
		return;
	}

	float DistSq = FVector::DistSquared(BossEnemy->GetActorLocation(), TargetActor->GetActorLocation());
	
	TArray<FRRBossAttack> ValidAttacks;
	float TotalWeight = 0.0f;
	
	for (const FRRBossAttack& Attack : BossEnemy->Attacks)
	{
		if (BossEnemy->GetPhase() >= Attack.MinPhase && DistSq <= (Attack.Range * Attack.Range))
		{
			ValidAttacks.Add(Attack);
			TotalWeight += Attack.Weight;
		}
	}
	
	if (ValidAttacks.Num() > 0)
	{
		float Rand = FMath::FRandRange(0.0f, TotalWeight);
		FRRBossAttack* Chosen = nullptr;
		for (FRRBossAttack& Attack : ValidAttacks)
		{
			Rand -= Attack.Weight;
			if (Rand <= 0.0f)
			{
				Chosen = &Attack;
				break;
			}
		}
		
		if (!Chosen) Chosen = &ValidAttacks.Last();

		if (bIsBackToBackPerilous)
		{
			// Pick a perilous attack if possible
			for (FRRBossAttack& Attack : ValidAttacks)
			{
				if (Attack.Kind != ERRBossAttackKind::Normal)
				{
					Chosen = &Attack;
					break;
				}
			}
			bIsBackToBackPerilous = false;
		}
		else if (BossEnemy->GetPhase() == 2 && Chosen->Kind != ERRBossAttackKind::Normal && FMath::FRand() < 0.4f)
		{
			bIsBackToBackPerilous = true;
		}
		
		BossEnemy->PlayAttackMontage(*Chosen);
		
		float MontageLen = 1.0f;
		if (Chosen->Montage.LoadSynchronous()) MontageLen = Chosen->Montage.Get()->GetPlayLength() / Chosen->PlayRate;
		else if (Chosen->Sequence.LoadSynchronous()) MontageLen = Chosen->Sequence.Get()->GetPlayLength() / Chosen->PlayRate;
		
		BossAttackTimer = GetWorld()->GetTimeSeconds() + MontageLen + Chosen->RecoverySeconds;
		PressureAttacksRemaining--;
	}
}
