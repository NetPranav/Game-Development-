// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAACraftingMaterialSettings.generated.h"

/**
 * UAAACraftingMaterialSettings
 *
 * Specialized Item Definition for crafting materials.
 * Lightweight. These items exist primarily for recipes and are always stackable.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAACraftingMaterialSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAACraftingMaterialSettings();

	/** Tier of the crafting material (1 = basic ore, 5 = divine essence). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Crafting", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaterialTier = 1;
};
