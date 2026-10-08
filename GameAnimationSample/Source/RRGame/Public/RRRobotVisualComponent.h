// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RRRobotVisualComponent.generated.h"

class UChildActorComponent;
class USkeletalMeshComponent;

/**
 * Swaps the player's visible body for a retargeted "display actor" (e.g. the Fab robot).
 *
 * The Game Animation Sample drives its locomotion on the UEFN mannequin and shows other characters through a
 * `VisualOverride` child actor whose Anim Blueprint retargets the master pose. Its own selection logic lives in
 * Blueprint graphs, so this component re-applies our chosen class shortly after BeginPlay (and once more later, to
 * win any race against the sample's default) and hides the master mannequin while still evaluating its pose.
 */
UCLASS(ClassGroup = (RR), meta = (BlueprintSpawnableComponent))
class RRGAME_API URRRobotVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** The display actor class to show instead of the mannequin. Falls back to URRSettings::PlayerVisualClass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Visual")
	TSubclassOf<AActor> VisualClass;

	UFUNCTION(BlueprintCallable, Category = "RR|Visual")
	void Apply();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTimerHandle ApplyTimerA;
	FTimerHandle ApplyTimerB;
};
