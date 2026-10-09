// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RRCinematicLook.generated.h"

class UPostProcessComponent;

/**
 * Runtime "film lens" post process layered over the level's own volume (it never touches exposure or GI, so the
 * environment artist's lighting stays in charge): soft bloom, light vignette, subtle grain and edge fringing, reduced
 * motion blur, and a gentle depth of field that keeps the player in focus and softens the far background.
 * Console: rr.look 0/1, rr.look.fstop <f>.
 */
UCLASS(NotPlaceable)
class RRGAME_API ARRCinematicLook : public AActor
{
	GENERATED_BODY()

public:
	ARRCinematicLook();
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "RR|Look")
	TObjectPtr<UPostProcessComponent> PostProcess;

private:
	float FocusDistance = 450.0f;
};
