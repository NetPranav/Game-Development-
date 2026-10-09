// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RRArena.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * The duel platform. Place one on the fight platform (actor origin = centre of the fighting floor).
 *  - Invisible circular wall (pawns only) keeps both fighters in and every NPC out.
 *  - Entry circle (EntryPoint, outside the wall): stand in it for EntryHoldSeconds -> teleported onto PlayerSpot, duel starts.
 *  - After the duel the player is put back at ExitPoint.
 *  - The boss first walks from BossEntrance to BossSpot (its entrance after the intro).
 * All points are components: move them in the level editor.
 */
UCLASS()
class RRGAME_API ARRArena : public AActor
{
	GENERATED_BODY()

public:
	ARRArena();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Radius of the fighting floor (wall ring). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Arena")
	float Radius = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Arena")
	float WallHeight = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Arena")
	float EntryRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Arena")
	float EntryHoldSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Arena")
	FLinearColor EntryColor = FLinearColor(0.25f, 0.75f, 1.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> PlayerSpot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> BossSpot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> EntryPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> ExitPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<USceneComponent> BossEntrance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<UStaticMeshComponent> EntryDisc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RR|Arena")
	TObjectPtr<UStaticMeshComponent> EntryGlow;

	FVector GetCenter() const { return GetActorLocation(); }
	bool IsInside(const FVector& Location, float Margin = 0.0f) const;

	/** 0..1 while the player stands in the circle. */
	float GetEntryProgress() const { return EntryProgress; }
	/** Player is near enough to show the prompt. */
	bool IsPlayerNearEntry() const { return bPlayerNear; }
	void SetEntryEnabled(bool bEnabled);
	bool IsEntryEnabled() const { return bEntryEnabled; }

	/** Boss passes through the walls while it walks in. */
	void SetActorPassesWalls(AActor* Actor, bool bPasses);

private:
	void BuildWalls();
	void SnapToGround();
	void UpdateEntryVisual(float RealDelta);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> Walls;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DiscMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GlowMID;

	float EntryProgress = 0.0f;
	bool bEntryEnabled = true;
	bool bPlayerNear = false;
	bool bTriggered = false;
	float Pulse = 0.0f;
};
