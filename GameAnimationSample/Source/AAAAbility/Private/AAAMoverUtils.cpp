// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAMoverUtils.h"
#include "MoverComponent.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

bool UAAAMoverUtils::ApplyVelocityBurst(AActor* Pawn, FVector WorldVelocity, float DurationSeconds)
{
	if (!Pawn)
	{
		return false;
	}

	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		Character->LaunchCharacter(WorldVelocity, true, true);
		return true;
	}

	// Mover pawns: queue velocity effects through the Mover simulation (SetActorLocation would fight it and rubber-band).
	// The velocity is re-applied every few frames with a falling ramp so ground friction cannot eat the push.
	UWorld* World = Pawn->GetWorld();
	if (!World || !Pawn->FindComponentByClass<UMoverComponent>())
	{
		return false;
	}

	auto QueueOnce = [](AActor* P, const FVector& Velocity)
	{
		if (UMoverComponent* Mover = P ? P->FindComponentByClass<UMoverComponent>() : nullptr)
		{
			TSharedPtr<FApplyVelocityEffect> Effect = MakeShared<FApplyVelocityEffect>();
			Effect->VelocityToApply = Velocity;
			Effect->bAdditiveVelocity = false;
			Effect->ForceMovementMode = NAME_None;
			Mover->QueueInstantMovementEffect(Effect);
		}
	};
	QueueOnce(Pawn, WorldVelocity);

	TSharedPtr<FTimerHandle> Handle = MakeShared<FTimerHandle>();
	TWeakObjectPtr<UWorld> WeakWorld(World);
	TWeakObjectPtr<AActor> WeakPawn(Pawn);
	const double Start = World->GetRealTimeSeconds(); // real time: hit-stop must not stretch the push
	World->GetTimerManager().SetTimer(*Handle, FTimerDelegate::CreateLambda([WeakWorld, WeakPawn, WorldVelocity, DurationSeconds, Start, Handle, QueueOnce]()
	{
		UWorld* W = WeakWorld.Get();
		AActor* P = WeakPawn.Get();
		const float T = W ? (float)(W->GetRealTimeSeconds() - Start) : DurationSeconds;
		if (!W || !P || T >= DurationSeconds)
		{
			if (W) { W->GetTimerManager().ClearTimer(*Handle); }
			return;
		}
		QueueOnce(P, WorldVelocity * (1.0f - T / DurationSeconds));
	}), 0.03f, true);
	return true;
}

FVector UAAAMoverUtils::GetPlanarVelocity(AActor* Pawn)
{
	if (!Pawn)
	{
		return FVector::ZeroVector;
	}

	FVector Vel = FVector::ZeroVector;
	if (UMoverComponent* MoverComp = Pawn->FindComponentByClass<UMoverComponent>())
	{
		Vel = MoverComp->GetVelocity();
	}
	else
	{
		Vel = Pawn->GetVelocity();
	}

	Vel.Z = 0.0f;
	return Vel;
}

FVector UAAAMoverUtils::GetMoveIntent(AActor* Pawn)
{
	if (!Pawn)
	{
		return FVector::ZeroVector;
	}

	if (UMoverComponent* MoverComp = Pawn->FindComponentByClass<UMoverComponent>())
	{
		return MoverComp->GetMovementIntent();
	}
	else
	{
		FVector Vel = GetPlanarVelocity(Pawn);
		if (!Vel.IsNearlyZero())
		{
			Vel.Normalize();
			return Vel;
		}
	}

	return FVector::ZeroVector;
}
