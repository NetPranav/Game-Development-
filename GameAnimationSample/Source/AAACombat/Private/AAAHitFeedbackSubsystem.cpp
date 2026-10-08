// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAHitFeedbackSubsystem.h"
#include "AAACombatEvents.h"
#include "AAACombatSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "TimerManager.h"
#include "Engine/HitResult.h"
#include "Shakes/WaveOscillatorCameraShakePattern.h"

UAAACombatCameraShake::UAAACombatCameraShake(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bSingleInstance = false;
	UWaveOscillatorCameraShakePattern* Pattern = CreateDefaultSubobject<UWaveOscillatorCameraShakePattern>(TEXT("RootPattern"));
	Pattern->LocationAmplitudeMultiplier = 1.f;
	Pattern->LocationFrequencyMultiplier = 1.f;
	Pattern->X.Amplitude = 1.f;
	Pattern->X.Frequency = 20.f;
	Pattern->Y.Amplitude = 1.f;
	Pattern->Y.Frequency = 20.f;
	Pattern->Z.Amplitude = 1.f;
	Pattern->Z.Frequency = 20.f;
	
	Pattern->Duration = 0.2f;
	SetRootShakePattern(Pattern);
}

void UAAAHitFeedbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_DedicatedServer)
	{
		HitConfirmedHandle = FAAACombatEvents::OnHitConfirmed().AddUObject(this, &UAAAHitFeedbackSubsystem::OnHitConfirmed);
		BlockedHandle = FAAACombatEvents::OnBlocked().AddUObject(this, &UAAAHitFeedbackSubsystem::OnBlocked);
		ParriedHandle = FAAACombatEvents::OnParried().AddUObject(this, &UAAAHitFeedbackSubsystem::OnParried);
	}
}

void UAAAHitFeedbackSubsystem::Deinitialize()
{
	if (HitConfirmedHandle.IsValid())
	{
		FAAACombatEvents::OnHitConfirmed().Remove(HitConfirmedHandle);
	}
	if (BlockedHandle.IsValid())
	{
		FAAACombatEvents::OnBlocked().Remove(BlockedHandle);
	}
	if (ParriedHandle.IsValid())
	{
		FAAACombatEvents::OnParried().Remove(ParriedHandle);
	}

	UWorld* World = GetWorld();
	if (World)
	{
		for (auto& Pair : ActiveHitStops)
		{
			if (AActor* A = Pair.Key.Get())
			{
				A->CustomTimeDilation = 1.0f;
			}
			World->GetTimerManager().ClearTimer(Pair.Value.TimerHandle);
		}
	}
	ActiveHitStops.Empty();

	Super::Deinitialize();
}

bool UAAAHitFeedbackSubsystem::IsLocalPlayer(AActor* InActor) const
{
	if (!InActor)
	{
		return false;
	}
	
	APawn* Pawn = Cast<APawn>(InActor);
	if (!Pawn)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	return PC && PC->IsLocalController();
}

void UAAAHitFeedbackSubsystem::OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	if (IsLocalPlayer(Attacker) || IsLocalPlayer(Victim))
	{
		ApplyHitStop(Attacker, Victim, bHeavy);
		PlayCameraShake(Attacker, Victim, bHeavy, Damage);
	}
	
	FVector Loc = Hit.ImpactPoint;
	if (Loc.IsZero() && Victim)
	{
		Loc = Victim->GetActorLocation();
	}
	AddDamageNumber(Loc, Damage, bHeavy ? FLinearColor(1.0f, 0.5f, 0.0f) : FLinearColor::White); // Orange heavy, White normal
}

void UAAAHitFeedbackSubsystem::OnBlocked(AActor* Attacker, AActor* Victim)
{
	if (Victim)
	{
		AddDamageNumber(Victim->GetActorLocation(), 0.0f, FLinearColor::Gray);
	}
}

void UAAAHitFeedbackSubsystem::OnParried(AActor* Attacker, AActor* Victim)
{
	if (Victim)
	{
		AddDamageNumber(Victim->GetActorLocation(), 0.0f, FLinearColor(0.0f, 1.0f, 1.0f)); // Cyan parried
	}
}

void UAAAHitFeedbackSubsystem::ApplyHitStop(AActor* Attacker, AActor* Victim, bool bHeavy)
{
	const UAAACombatSettings* Settings = GetDefault<UAAACombatSettings>();
	if (!Settings || !Settings->bEnableHitStop)
	{
		return;
	}

	float Duration = bHeavy ? Settings->HitStopHeavySeconds : Settings->HitStopLightSeconds;
	if (Duration <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	auto FreezeActor = [this, Duration, World](AActor* InActor)
	{
		if (!InActor) return;
		
		InActor->CustomTimeDilation = 0.03f;
		
		FHitStopEntry& Entry = ActiveHitStops.FindOrAdd(InActor);
		TWeakObjectPtr<AActor> WeakActor(InActor);
		
		World->GetTimerManager().SetTimer(Entry.TimerHandle, [this, WeakActor]()
		{
			if (AActor* A = WeakActor.Get())
			{
				A->CustomTimeDilation = 1.0f;
			}
			ActiveHitStops.Remove(WeakActor);
		}, Duration, false);
	};

	FreezeActor(Attacker);
	FreezeActor(Victim);
}

void UAAAHitFeedbackSubsystem::PlayCameraShake(AActor* Attacker, AActor* Victim, bool bHeavy, float Damage)
{
	const UAAACombatSettings* Settings = GetDefault<UAAACombatSettings>();
	if (!Settings || !Settings->bEnableCameraShake)
	{
		return;
	}

	AActor* LocalActor = nullptr;
	if (IsLocalPlayer(Attacker)) LocalActor = Attacker;
	else if (IsLocalPlayer(Victim)) LocalActor = Victim;

	if (!LocalActor) return;

	APawn* Pawn = Cast<APawn>(LocalActor);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	float Scale = bHeavy ? Settings->CameraShakeHeavyScale : Settings->CameraShakeLightScale;
	Scale *= FMath::Max(1.0f, Damage / 10.0f); // Arbitrary scaling by damage

	PC->ClientStartCameraShake(UAAACombatCameraShake::StaticClass(), Scale);
}

void UAAAHitFeedbackSubsystem::AddDamageNumber(const FVector& Location, float Damage, const FLinearColor& Color)
{
	FDamageNumberEntry Entry;
	Entry.WorldLocation = Location;
	Entry.Damage = Damage;
	Entry.LifeTime = 0.0f;
	Entry.Color = Color;
	Entry.bActive = true;

	DamageNumbers[DamageNumberHead] = Entry;
	DamageNumberHead = (DamageNumberHead + 1) % MaxDamageNumbers;
	if (DamageNumberCount < MaxDamageNumbers)
	{
		DamageNumberCount++;
	}
}

void UAAAHitFeedbackSubsystem::DrawDamageNumbers(AHUD* HUD)
{
	if (!HUD || DamageNumberCount == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	float DeltaTime = World->GetDeltaSeconds();
	UFont* Font = GEngine->GetMediumFont();

	for (int32 i = 0; i < MaxDamageNumbers; ++i)
	{
		FDamageNumberEntry& Entry = DamageNumbers[i];
		if (!Entry.bActive) 
		{
			continue;
		}

		Entry.LifeTime += DeltaTime;
		if (Entry.LifeTime >= 0.9f)
		{
			Entry.bActive = false;
			continue; // Just expired
		}

		float Alpha = 1.0f - (Entry.LifeTime / 0.9f);
		FVector CurrentLoc = Entry.WorldLocation + FVector(0.0f, 0.0f, (Entry.LifeTime / 0.9f) * 60.0f);
		
		FVector ScreenLoc = HUD->Project(CurrentLoc);
		if (ScreenLoc.Z > 0.0f) // Behind camera check
		{
			FString Text = (Entry.Damage > 0.0f) ? FString::Printf(TEXT("%d"), FMath::RoundToInt(Entry.Damage)) : ((Entry.Color == FLinearColor(0.0f, 1.0f, 1.0f)) ? TEXT("Parried") : TEXT("Blocked"));
			
			FLinearColor DrawColor = Entry.Color;
			DrawColor.A = Alpha;
			// AHUD::Canvas is protected; the public DrawText wrapper is enough here.
			HUD->DrawText(Text, DrawColor, ScreenLoc.X, ScreenLoc.Y, Font, 1.25f, false);
		}
	}
}
