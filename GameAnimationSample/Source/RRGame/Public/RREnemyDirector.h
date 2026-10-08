// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RREnemyDirector.generated.h"

class ARREnemy;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRROnEnemySpawned, ARREnemy*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRROnEnemyDied, ARREnemy*, Enemy, int32, RemainingLiving);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRROnAllEnemiesDefeated);

/**
 * Per-world registry of living enemies plus the "attack token" scheduler that limits how many
 * enemies attack at the same time. Runs on the server / standalone only; enemies are discovered
 * automatically (existing ones at world start, later ones through the actor-spawned callback).
 */
UCLASS()
class RRGAME_API URREnemyDirector : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** Spawns an enemy (snapped to the ground) and registers it. Returns null on failure. */
	UFUNCTION(BlueprintCallable, Category = "RR|Enemy")
	ARREnemy* SpawnEnemy(TSubclassOf<ARREnemy> EnemyClass, const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "RR|Enemy")
	TArray<ARREnemy*> GetLivingEnemies() const;

	UFUNCTION(BlueprintPure, Category = "RR|Enemy")
	int32 GetLivingEnemyCount() const;

	/** Called by ARREnemy when it dies. */
	void NotifyEnemyDied(ARREnemy* Enemy);

	/** Attack tokens: at most URRSettings::MaxSimultaneousAttackers holders at a time. */
	bool TryAcquireAttackToken(AController* Requester);
	void ReleaseAttackToken(AController* Requester);

	UPROPERTY(BlueprintAssignable, Category = "RR|Enemy")
	FRROnEnemySpawned OnEnemySpawned;

	UPROPERTY(BlueprintAssignable, Category = "RR|Enemy")
	FRROnEnemyDied OnEnemyDied;

	UPROPERTY(BlueprintAssignable, Category = "RR|Enemy")
	FRROnAllEnemiesDefeated OnAllEnemiesDefeated;

private:
	void Register(ARREnemy* Enemy);
	void Unregister(ARREnemy* Enemy);
	void HandleActorSpawned(AActor* Actor);
	void PurgeTokens();

	TArray<TWeakObjectPtr<ARREnemy>> Enemies;
	TArray<TWeakObjectPtr<AController>> TokenHolders;
	FDelegateHandle ActorSpawnedHandle;
};
