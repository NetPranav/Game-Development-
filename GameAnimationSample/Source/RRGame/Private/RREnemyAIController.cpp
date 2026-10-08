// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RREnemyAIController.h"
#include "RRGame.h"
#include "RREnemy.h"
#include "RREnemyDirector.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AAACombatComponent.h"
#include "AAAAttributeComponent.h"
#include "AAAGameplayTags.h"
#include "AAAAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AAAGameplayTags.h"
namespace
{
	/** Generic "is this pawn still alive" for any pawn with an ASC (the player is not an ARREnemy). */
	bool IsPawnAlive(const APawn* Pawn)
	{
		if (!Pawn)
		{
			return false;
		}
		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
		{
			if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
				return !ASC->HasMatchingGameplayTag(Tags.State_Combat_Dead) && !ASC->HasMatchingGameplayTag(Tags.State_Dead);
			}
		}
		return true;
	}
}


static TAutoConsoleVariable<int32> CVarRRAIDebug(
	TEXT("rr.ai.debug"),
	0,
	TEXT("Draw debug info for RR AI.\n")
	TEXT("0: Disable, 1: Enable"),
	ECVF_Cheat);

ARREnemyAIController::ARREnemyAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = false;
}

void ARREnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ARREnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseToken();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ThinkTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ARREnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ControlledEnemy = Cast<ARREnemy>(InPawn);
	CurrentState = ERREnemyState::Idle;
	UE_LOG(LogRR, Log, TEXT("AI possessed %s (authority=%d)"), *GetNameSafe(InPawn), HasAuthority() ? 1 : 0);
	
	if (ControlledEnemy.IsValid() && HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ThinkTimerHandle,
			this,
			&ARREnemyAIController::ThinkLoop,
			0.1f,
			true
		);
	}
}

void ARREnemyAIController::OnUnPossess()
{
	ReleaseToken();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ThinkTimerHandle);
	}
	ControlledEnemy.Reset();
	Super::OnUnPossess();
}

void ARREnemyAIController::TransitionTo(ERREnemyState NewState)
{
	if (CurrentState == NewState) return;
	
	// Exit state
	if (CurrentState == ERREnemyState::Chase || CurrentState == ERREnemyState::Attack)
	{
		StopMovement();
	}
	
	UE_LOG(LogRR, Log, TEXT("AI %s: state %d -> %d"), *GetNameSafe(GetPawn()), (int32)CurrentState, (int32)NewState);
	CurrentState = NewState;
	
	// Enter state
	if (CurrentState == ERREnemyState::Recover)
	{
		if (ControlledEnemy.IsValid())
		{
			float MinCD = ControlledEnemy->Tuning.AttackCooldownMin;
			float MaxCD = ControlledEnemy->Tuning.AttackCooldownMax;
			RecoveryEndTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(MinCD, MaxCD);
		}
	}
}

void ARREnemyAIController::ReleaseToken()
{
	if (bHasAttackToken)
	{
		if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
		{
			Director->ReleaseAttackToken(this);
		}
		bHasAttackToken = false;
	}
}

void ARREnemyAIController::AcquireTarget()
{
	if (!ControlledEnemy.IsValid()) return;
	
	float Aggro = ControlledEnemy->Tuning.AggroRange;
	float Lose = ControlledEnemy->Tuning.LoseRange;
	
	// Check if current target is still valid
	if (TargetActor.IsValid())
	{
		APawn* TargetPawn = Cast<APawn>(TargetActor.Get());
		if (!TargetPawn || !IsPawnAlive(TargetPawn) || !TargetPawn->GetController())
		{
			TargetActor.Reset();
		}
		else
		{
			float DistSq = FVector::DistSquared(ControlledEnemy->GetActorLocation(), TargetActor->GetActorLocation());
			if (DistSq > Lose * Lose)
			{
				TargetActor.Reset();
			}
			else
			{
				// Still valid
				return;
			}
		}
	}
	
	// Find new target
	AActor* BestTarget = nullptr;
	float BestDistSq = Aggro * Aggro;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* P = PC->GetPawn();
			// Check if alive using AAA components or simply if it has State.Combat.Dead
			bool bIsDead = false;
			if (UAAAAbilitySystemComponent* TargetASC = P->FindComponentByClass<UAAAAbilitySystemComponent>())
			{
				if (TargetASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
				{
					bIsDead = true;
				}
			}
			
			if (!bIsDead)
			{
				float DistSq = FVector::DistSquared(ControlledEnemy->GetActorLocation(), P->GetActorLocation());
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestTarget = P;
				}
			}
		}
	}
	
	if (BestTarget && BestTarget != TargetActor.Get())
	{
		UE_LOG(LogRR, Log, TEXT("AI %s acquired target %s"), *GetNameSafe(GetPawn()), *GetNameSafe(BestTarget));
	}
	TargetActor = BestTarget;
}

void ARREnemyAIController::HandleStaggerAndReactions()
{
	if (!ControlledEnemy.IsValid()) return;
	
	UAAACombatComponent* CombatComp = ControlledEnemy->CombatComponent;
	
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
		// Tag cleared
		TransitionTo(ERREnemyState::Idle);
	}
	
	// Player attack reaction
	if (TargetActor.IsValid() && CombatComp && CurrentState != ERREnemyState::Attack && CurrentState != ERREnemyState::Staggered && CurrentState != ERREnemyState::Dead)
	{
		float DistSq = FVector::DistSquared(ControlledEnemy->GetActorLocation(), TargetActor->GetActorLocation());
		if (DistSq <= 350.0f * 350.0f)
		{
			UAAACombatComponent* TargetCombat = TargetActor->FindComponentByClass<UAAACombatComponent>();
			if (TargetCombat && TargetCombat->IsAttacking())
			{
				float TimeSecs = GetWorld()->GetTimeSeconds();
				if (TimeSecs - LastSidestepTime > 3.0f)
				{
					if (FMath::FRand() < ControlledEnemy->Tuning.SidestepChance)
					{
						LastSidestepTime = TimeSecs;
						FVector DirAway = (ControlledEnemy->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
						ControlledEnemy->LaunchCharacter(DirAway * 600.0f, true, true);
						// Small wait time to complete sidestep
					}
					else if (FMath::FRand() < ControlledEnemy->Tuning.BlockChanceWhenPlayerAttacking)
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

void ARREnemyAIController::ThinkLoop()
{
	if (!ControlledEnemy.IsValid() || !ControlledEnemy->IsAlive())
	{
		if (CurrentState != ERREnemyState::Dead)
		{
			ReleaseToken();
			StopMovement();
			TransitionTo(ERREnemyState::Dead);
		}
		return;
	}
	
	AcquireTarget();
	HandleStaggerAndReactions();

#if !UE_BUILD_SHIPPING
	// Periodic diagnostics (every ~2 s per enemy) so headless/automated runs can see why an enemy is stuck.
	if (CVarRRAIDebug.GetValueOnGameThread() > 1 && GetWorld()->GetTimeSeconds() >= NextDiagTime)
	{
		NextDiagTime = GetWorld()->GetTimeSeconds() + 2.0f;
		const float Dist = TargetActor.IsValid() ? FVector::Dist(ControlledEnemy->GetActorLocation(), TargetActor->GetActorLocation()) : -1.f;
		UE_LOG(LogRR, Log, TEXT("AI-DIAG %s state=%d dist=%.0f speed=%.0f moveMode=%d pathStatus=%d loc=%s"),
			*GetNameSafe(ControlledEnemy.Get()), (int32)CurrentState, Dist, ControlledEnemy->GetVelocity().Size(),
			ControlledEnemy->GetCharacterMovement() ? (int32)ControlledEnemy->GetCharacterMovement()->MovementMode : -1,
			(int32)GetMoveStatus(), *ControlledEnemy->GetActorLocation().ToCompactString());
	}
#endif
	
	if (CurrentState == ERREnemyState::Dead || CurrentState == ERREnemyState::Staggered)
	{
		return;
	}
	
#if !UE_BUILD_SHIPPING
	if (CVarRRAIDebug.GetValueOnGameThread() > 0)
	{
		FString StateLabel = TEXT("Idle");
		switch(CurrentState)
		{
			case ERREnemyState::Chase: StateLabel = TEXT("Chase"); break;
			case ERREnemyState::Circle: StateLabel = TEXT("Circle"); break;
			case ERREnemyState::Attack: StateLabel = TEXT("Attack"); break;
			case ERREnemyState::Recover: StateLabel = TEXT("Recover"); break;
			default: break;
		}
		DrawDebugString(GetWorld(), ControlledEnemy->GetActorLocation() + FVector(0,0,100), StateLabel, nullptr, FColor::Yellow, 0.1f);
	}
#endif

	if (!TargetActor.IsValid())
	{
		TransitionTo(ERREnemyState::Idle);
		return;
	}

	FVector MyLoc = ControlledEnemy->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();
	float DistToTarget = FVector::Dist2D(MyLoc, TargetLoc);
	FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal2D();
	
	// Face target in combat range
	if (DistToTarget <= ControlledEnemy->Tuning.AggroRange)
	{
		ControlledEnemy->bUseControllerRotationYaw = false;
		if (UCharacterMovementComponent* MoveComp = ControlledEnemy->GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = false;
		}
		FRotator TargetRot = DirToTarget.Rotation();
		FRotator NewRot = FMath::RInterpTo(ControlledEnemy->GetActorRotation(), TargetRot, 0.1f, 10.0f);
		ControlledEnemy->SetActorRotation(NewRot);
	}
	
	if (CurrentState == ERREnemyState::Idle)
	{
		TransitionTo(ERREnemyState::Chase);
	}
	
	if (CurrentState == ERREnemyState::Chase)
	{
		if (DistToTarget <= ControlledEnemy->Tuning.PreferredCircleRange)
		{
			TransitionTo(ERREnemyState::Circle);
		}
		else
		{
			HandleMovement();
		}
	}
	
	if (CurrentState == ERREnemyState::Circle)
	{
		if (DistToTarget > ControlledEnemy->Tuning.PreferredCircleRange * 1.5f)
		{
			TransitionTo(ERREnemyState::Chase);
		}
		else
		{
			// Strafe
			float TimeSecs = GetWorld()->GetTimeSeconds();
			if (TimeSecs - LastCircleChangeTime > FMath::RandRange(1.5f, 3.0f))
			{
				CurrentCircleDirection *= -1.0f;
				LastCircleChangeTime = TimeSecs;
			}
			
			FVector Tangent = FVector::CrossProduct(FVector::UpVector, DirToTarget) * CurrentCircleDirection;
			ControlledEnemy->AddMovementInput(Tangent);
			
			// Try to attack
			if (!bHasAttackToken)
			{
				if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
				{
					if (Director->TryAcquireAttackToken(this))
					{
						bHasAttackToken = true;
						TransitionTo(ERREnemyState::Attack);
					}
				}
			}
		}
	}
	
	if (CurrentState == ERREnemyState::Attack)
	{
		if (DistToTarget > ControlledEnemy->Tuning.AttackRange)
		{
			HandleMovement(); // Move closer
		}
		else
		{
			// Close enough. Face within 25 degrees.
			float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ControlledEnemy->GetActorForwardVector(), DirToTarget)));
			if (Angle <= 25.0f)
			{
				HandleCombat();
			}
			else
			{
				// Still rotating, wait. Add slight movement towards target
				ControlledEnemy->AddMovementInput(DirToTarget);
			}
		}
	}
	
	if (CurrentState == ERREnemyState::Recover)
	{
		if (GetWorld()->GetTimeSeconds() >= RecoveryEndTime)
		{
			ReleaseToken();
			TransitionTo(ERREnemyState::Circle);
		}
		else
		{
			// Slowly back away or circle
			ControlledEnemy->AddMovementInput(-DirToTarget * 0.5f);
		}
	}
}

void ARREnemyAIController::HandleMovement()
{
	if (!ControlledEnemy.IsValid() || !TargetActor.IsValid()) return;
	
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys && NavSys->GetMainNavData())
	{
		// bStopOnOverlap=false: with true the goal radius also includes both capsule radii, so enemies
		// halted ~40 cm outside AttackRange and never started their attack.
		MoveToActor(TargetActor.Get(), ControlledEnemy->Tuning.AttackRange * 0.6f, /*bStopOnOverlap*/ false, /*bUsePathfinding*/ true, /*bCanStrafe*/ true);
	}
	else
	{
		FVector DirToTarget = (TargetActor->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal2D();
		ControlledEnemy->AddMovementInput(DirToTarget);
	}
}

void ARREnemyAIController::HandleCombat()
{
	if (!ControlledEnemy.IsValid() || !TargetActor.IsValid()) return;
	
	UAAACombatComponent* CombatComp = ControlledEnemy->CombatComponent;
	if (!CombatComp) return;
	
	if (CombatComp->IsAttacking())
	{
		// Wait until done
		if (GetWorld()->GetTimeSeconds() - AttackStartTime > 3.0f)
		{
			TransitionTo(ERREnemyState::Recover);
		}
	}
	else
	{
		if (AttackStartTime > 0.0f && GetWorld()->GetTimeSeconds() - AttackStartTime > 0.1f)
		{
			// Just finished attacking
			AttackStartTime = 0.0f;
			TransitionTo(ERREnemyState::Recover);
		}
		else if (AttackStartTime == 0.0f)
		{
			// Start attack
			EAAAAttackType Type = EAAAAttackType::Light;
			if (FMath::FRand() < ControlledEnemy->Tuning.LightVsHeavy)
			{
				Type = EAAAAttackType::Heavy;
			}
			CombatComp->RequestAttack(Type);
			AttackStartTime = GetWorld()->GetTimeSeconds();
		}
	}
}
