// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRNpcController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "RRArena.h"
#include "RRGameMode.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"

ARRNpcController::ARRNpcController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARRNpcController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Home = InPawn ? InPawn->GetActorLocation() : FVector::ZeroVector;
	NextDecisionTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(1.0f, 4.0f);
}

void ARRNpcController::PickNextGoal()
{
	APawn* P = GetPawn();
	if (!P) return;

	FVector2D Offset = FMath::RandPointInCircle(WanderRadius);
	Goal = Home + FVector(Offset.X, Offset.Y, 0.0f);
	if (Arena.IsValid())
	{
		// never into the platform: push the goal out to the walkway around it
		const FVector C = Arena->GetCenter();
		const float Keep = Arena->Radius + 380.0f;
		FVector Out = Goal - C; Out.Z = 0.0f;
		if (Out.Size() < Keep) { Goal = C + Out.GetSafeNormal() * (Keep + FMath::FRandRange(0.0f, 300.0f)) + FVector(0, 0, Goal.Z - C.Z); }
	}
	bMoving = true;
	bDirectSteer = true;
	MoveStartTime = GetWorld()->GetTimeSeconds();
	ClearFocus(EAIFocusPriority::Gameplay);

	if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation OnNav;
		if (Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) && Nav->ProjectPointToNavigation(Goal, OnNav, FVector(200.0f, 200.0f, 500.0f)))
		{
			Goal = OnNav.Location;
			bDirectSteer = MoveToLocation(Goal, 60.0f) == EPathFollowingRequestResult::Failed;
		}
	}
}

void ARRNpcController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	APawn* P = GetPawn();
	if (!P) return;

	const double Now = GetWorld()->GetTimeSeconds();
	if (bSpectator)
	{
		TickSpectator(P, Now);
		return;
	}
	if (Arena.IsValid() && Arena->IsInside(P->GetActorLocation(), 220.0f))
	{
		// somehow got close: walk straight back out
		FVector Out = P->GetActorLocation() - Arena->GetCenter(); Out.Z = 0.0f;
		P->AddMovementInput(Out.GetSafeNormal(), 0.6f);
		return;
	}
	if (bMoving)
	{
		const float Dist = FVector::Dist2D(P->GetActorLocation(), Goal);
		const bool bPathDone = !bDirectSteer && GetMoveStatus() == EPathFollowingStatus::Idle;
		if (Dist < 80.0f || bPathDone || Now - MoveStartTime > 12.0)
		{
			// arrived: stand and watch the fight for a while
			bMoving = false;
			StopMovement();
			if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				SetFocus(Player, EAIFocusPriority::Gameplay);
			}
			NextDecisionTime = Now + FMath::FRandRange(3.0f, 8.0f);
		}
		else if (bDirectSteer)
		{
			P->AddMovementInput((Goal - P->GetActorLocation()).GetSafeNormal2D(), StrollInputScale);
		}
	}
	else if (Now >= NextDecisionTime)
	{
		PickNextGoal();
	}
}

void ARRNpcController::MakeSpectator(ARRArena* InArena)
{
	Arena = InArena;
	bSpectator = true;
	NextEmoteTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(1.0f, 4.0f);
}

void ARRNpcController::PlayMontage(APawn* P, const TCHAR* Path)
{
	ACharacter* C = Cast<ACharacter>(P);
	UAnimMontage* M = LoadObject<UAnimMontage>(nullptr, Path);
	if (C && M && !C->GetCurrentMontage())
	{
		C->PlayAnimMontage(M, FMath::FRandRange(0.9f, 1.1f));
	}
}

void ARRNpcController::TickSpectator(APawn* P, double Now)
{
	if (Arena.IsValid())
	{
		// face the duel (the sandbox character orients to movement, so turn the body directly while standing)
		const FRotator Want = (Arena->GetCenter() - P->GetActorLocation()).GetSafeNormal2D().Rotation();
		P->SetActorRotation(FMath::RInterpTo(P->GetActorRotation(), FRotator(0.0f, Want.Yaw, 0.0f), GetWorld()->GetDeltaSeconds(), 3.0f));
	}
	// stay on the spot
	if (FVector::Dist2D(P->GetActorLocation(), Home) > 90.0f)
	{
		P->AddMovementInput((Home - P->GetActorLocation()).GetSafeNormal2D(), 0.4f);
		return;
	}
	if (Now < NextEmoteTime) return;

	const ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
	if (GM && GM->IsDuelActive())
	{
		PlayMontage(P, TEXT("/Game/RR/NPC/AM_NPC_Dance.AM_NPC_Dance"));   // cheering for the fight
		NextEmoteTime = Now + FMath::FRandRange(2.5f, 5.0f);
	}
	else
	{
		PlayMontage(P, TEXT("/Game/RR/NPC/AM_NPC_Talk.AM_NPC_Talk"));
		NextEmoteTime = Now + FMath::FRandRange(6.0f, 12.0f);
	}
}
