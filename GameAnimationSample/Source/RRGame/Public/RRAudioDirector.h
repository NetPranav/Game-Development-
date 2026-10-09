// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RRAudioDirector.generated.h"

// Forward Declarations
class USoundBase;
class UAudioComponent;
struct FHitResult;
class AActor;
class ARRBoss;
enum class ERRBossAttackKind : uint8;

UENUM(BlueprintType)
enum class ERRMusicState : uint8
{
	None,
	Explore,
	Tension,
	Battle,
	Victory,
	Defeat
};

UCLASS()
class RRGAME_API URRAudioDirector : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable, Category = "RR|Audio")
	void SetMusicState(ERRMusicState NewState);

	UFUNCTION(BlueprintCallable, Category = "RR|Audio")
	ERRMusicState GetMusicState() const;

	UFUNCTION(BlueprintCallable, Category = "RR|Audio")
	void PlayCrowd(float Volume = 0.6f);

	UFUNCTION(BlueprintCallable, Category = "RR|Audio")
	void PlaySfxAt(USoundBase* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

private:
	void OnMusicCommand(const TArray<FString>& Args);

	void OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);
	void OnBlocked(AActor* Attacker, AActor* Victim);
	void OnParried(AActor* Attacker, AActor* Victim);
	void OnDeathblow(ARRBoss* Boss);
	void OnPerilousWarning(ARRBoss* Boss, ERRBossAttackKind Kind, float WarningSeconds);

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CurrentMusicComp;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FadingMusicComp;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AmbienceComp;

	ERRMusicState CurrentMusicState = ERRMusicState::None;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> MusicExplore;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> MusicTension;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> MusicBattle;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> AmbienceWind;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundBase>> SfxSwords;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundBase>> SfxClashes;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> SfxCrowd;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> SfxEffortSwing;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> SfxEffortCheer;

	bool bAmbienceStarted = false;
	double LastHitTime = 0.0;
	float MusicVolume = 0.0f;
	class IConsoleObject* MusicCmd = nullptr;
	FDelegateHandle HitHandle, BlockHandle, ParryHandle, DeathblowHandle, PerilousHandle;
};
