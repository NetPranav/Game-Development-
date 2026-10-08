// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AAAAWeaponActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UAAAWeaponSettings;

UCLASS()
class AAACOMBAT_API AAAAWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAAAWeaponActor();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Initializes the weapon mesh from the settings data asset */
	UFUNCTION(BlueprintCallable, Category = "AAA|Weapon")
	void InitializeWeapon(UAAAWeaponSettings* InWeaponSettings);

	/** Starts the continuous collision trace for attack sweeps */
	UFUNCTION(BlueprintCallable, Category = "AAA|Weapon")
	void StartWeaponTrace();

	/** Stops the collision trace and clears the hit array */
	UFUNCTION(BlueprintCallable, Category = "AAA|Weapon")
	void StopWeaponTrace();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Performs the actual sweep logic between last frame and current frame sockets */
	void PerformTrace();

	/** Sweep from the owner along its facing direction; used when the weapon has no mesh (weapon is part of the body). */
	void PerformBodySweep();

	/** Team/dead filtering + GameplayEvent.Combat.Hit dispatch shared by blade and body sweeps. */
	void ProcessHit(const FHitResult& Hit, AActor* MyOwner, class IGenericTeamAgentInterface* MyTeamAgent, TArray<AActor*>& ActorsToIgnore);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	/** The settings used to initialize this weapon */
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Weapon Data")
	TObjectPtr<UAAAWeaponSettings> WeaponSettings;

	/** Whether the weapon is currently actively tracing for hits */
	bool bIsTracing;

	/** Store the previous frame's socket location to sweep against */
	TArray<FVector> PreviousBladePositions;

	/** Keep track of actors already hit during a single swing to prevent multi-hitting */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActors;

	double LastFallbackWarnTime = 0.0;

	/** Optional: Name of the sockets for start and end of the blade. Defaults to generic names if none provided */
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FName TraceStartSocket = FName("TraceStart");
	
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FName TraceEndSocket = FName("TraceEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Weapon")
	float TraceRadius = 20.0f;

	/** Sphere radius used by the mesh-less body sweep. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Weapon|Trace")
	float BodySweepRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Weapon")
	int32 NumBladeSamples = 3;
};
