// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RRCinematicDirector.generated.h"

class ACameraActor;
class ARRBoss;
class URRCinematicSequence;
class UAAACombatComponent;
class USkeletalMeshComponent;
class UParticleSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRRCinematicSignature);

/**
 * Plays side-camera special-attack scenes (boss kick -> hero braces, slides back, plants its sword).
 * Driven by URRCinematicSequence data; started by the boss' special attack (FRRBossEvents) or by hero special moves.
 */
UCLASS()
class RRGAME_API URRCinematicDirector : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(URRCinematicDirector, STATGROUP_Tickables); }
	virtual bool IsTickableInEditor() const override { return false; }

	/** Starts a scene. SecondsToContact < 0 uses the sequence default. Returns false if one is running or an actor is dead/invalid. */
	UFUNCTION(BlueprintCallable, Category = "RR|Cinematic")
	bool PlaySequence(URRCinematicSequence* Sequence, AActor* Attacker, AActor* Defender, float SecondsToContact = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "RR|Cinematic")
	void Abort();

	UFUNCTION(BlueprintPure, Category = "RR|Cinematic")
	bool IsPlaying() const { return bActive; }

	/** 0..1, eased: how far the letterbox bars are in. Used by the HUD. */
	UFUNCTION(BlueprintPure, Category = "RR|Cinematic")
	float GetBarAlpha() const { return BarAlpha; }

	UPROPERTY(BlueprintAssignable, Category = "RR|Cinematic")
	FRRCinematicSignature OnCinematicStarted;

	UPROPERTY(BlueprintAssignable, Category = "RR|Cinematic")
	FRRCinematicSignature OnCinematicFinished;

private:
	void OnBossSpecialBegin(ARRBoss* Boss, AActor* Target, float SecondsToContact);
	void DoContact();
	void StartSlide();
	void Finish();
	void RestoreWorld();
	void UpdateCamera(float RealDelta);
	bool ComputeShot(int32 Index, float Time, FVector& OutLoc, FRotator& OutRot, float& OutFOV) const;
	float PlayAnimOnActor(AActor* Actor, const struct FRRCineAnim& Anim) const;
	USkeletalMeshComponent* FindVisibleMesh(AActor* Actor) const;
	void SpawnEffectAttached(UParticleSystem* Effect, AActor* Actor, FName Bone) const;

	bool bActive = false;
	bool bContactDone = false;
	bool bSlideStarted = false;
	bool bTimeDilated = false;
	bool bSlideAnimPlayed = false;
	bool bStopAnimPlayed = false;
	double Time = 0.0;
	float ContactTime = 0.9f;
	float EndTime = 3.0f;
	float DilationRestoreAt = -1.0f;
	float NextSlideSparkAt = 0.0f;
	float BarAlpha = 0.0f;

	FVector Axis = FVector::ForwardVector;
	FVector Right = FVector::RightVector;
	FVector StartDefenderLoc = FVector::ZeroVector;
	FVector LastCameraLoc = FVector::ZeroVector;
	FRotator LastCameraRot = FRotator::ZeroRotator;
	float LastCameraFOV = 60.0f;
	bool bHaveLastCamera = false;

	TWeakObjectPtr<AActor> Attacker;
	TWeakObjectPtr<AActor> Defender;
	TWeakObjectPtr<AActor> PreviousViewTarget;
	TWeakObjectPtr<ACameraActor> Camera;

	UPROPERTY(Transient)
	TObjectPtr<URRCinematicSequence> Active;

	/** Short global slow motion with half letterbox (deathblows). */
	void PlayMoment(float Dilation, float RealSeconds);
	bool bMoment = false;
	double MomentEnd = 0.0;
	FDelegateHandle DeathblowHandle;

	FDelegateHandle BossBeginHandle;
	FDelegateHandle BossContactHandle;
};
