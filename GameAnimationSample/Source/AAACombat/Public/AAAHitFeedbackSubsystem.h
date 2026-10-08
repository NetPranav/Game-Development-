// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/CameraShakeBase.h"
#include "AAAHitFeedbackSubsystem.generated.h"

class AActor;
class AHUD;
struct FHitResult;

UCLASS()
class UAAACombatCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()
public:
	UAAACombatCameraShake(const FObjectInitializer& ObjectInitializer);
};

USTRUCT()
struct FDamageNumberEntry
{
	GENERATED_BODY()

	FVector WorldLocation;
	float Damage;
	float LifeTime;
	FLinearColor Color;
	bool bActive;

	FDamageNumberEntry() : WorldLocation(ForceInit), Damage(0.f), LifeTime(0.f), Color(ForceInit), bActive(false) {}
};

struct FHitStopEntry
{
	FTimerHandle TimerHandle;
};

/**
 * AAAHitFeedbackSubsystem
 * Handles hit-stop, camera shake, and damage numbers.
 * Client-side only.
 */
UCLASS()
class AAACOMBAT_API UAAAHitFeedbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Draws damage numbers on the HUD */
	void DrawDamageNumbers(AHUD* HUD);

private:
	void OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);
	void OnBlocked(AActor* Attacker, AActor* Victim);
	void OnParried(AActor* Attacker, AActor* Victim);

	void ApplyHitStop(AActor* Attacker, AActor* Victim, bool bHeavy);
	void PlayCameraShake(AActor* Attacker, AActor* Victim, bool bHeavy, float Damage);
	void AddDamageNumber(const FVector& Location, float Damage, const FLinearColor& Color);

	bool IsLocalPlayer(AActor* InActor) const;

	FDelegateHandle HitConfirmedHandle;
	FDelegateHandle BlockedHandle;
	FDelegateHandle ParriedHandle;

	static const int32 MaxDamageNumbers = 32;
	FDamageNumberEntry DamageNumbers[MaxDamageNumbers];
	int32 DamageNumberHead = 0;
	int32 DamageNumberCount = 0;

	TMap<TWeakObjectPtr<AActor>, FHitStopEntry> ActiveHitStops;
};
