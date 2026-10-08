// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AAAMoverUtils.generated.h"

UCLASS()
class AAAABILITY_API UAAAMoverUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Applies a short burst of velocity to a Mover pawn, Character, or generic Actor */
	UFUNCTION(BlueprintCallable, Category = "AAA|Mover")
	static bool ApplyVelocityBurst(AActor* Pawn, FVector WorldVelocity, float DurationSeconds);

	/** Returns planar velocity */
	UFUNCTION(BlueprintPure, Category = "AAA|Mover")
	static FVector GetPlanarVelocity(AActor* Pawn);

	/** Returns movement intent */
	UFUNCTION(BlueprintPure, Category = "AAA|Mover")
	static FVector GetMoveIntent(AActor* Pawn);
};
