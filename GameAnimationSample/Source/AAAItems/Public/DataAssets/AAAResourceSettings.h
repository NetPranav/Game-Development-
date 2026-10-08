// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAAResourceSettings.generated.h"

/**
 * UAAAResourceSettings
 *
 * Specialized Item Definition for world resources (ores, wood, herbs, gems).
 * Highly stackable, lightweight, typically gathered from the open world.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAResourceSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAAResourceSettings();

	/** Gathering skill required (future use). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Resource")
	FName RequiredGatheringSkill;

	/** Minimum gathering level needed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Resource", meta = (ClampMin = "0"))
	int32 RequiredGatheringLevel = 0;

	/** Resource node type tag for world spawners. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Resource")
	FGameplayTag ResourceNodeTag;
};
