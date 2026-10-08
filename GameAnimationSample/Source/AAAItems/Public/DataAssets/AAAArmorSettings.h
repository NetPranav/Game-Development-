// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAAItemTypes.h"
#include "AAAArmorSettings.generated.h"

/**
 * UAAAArmorSettings
 *
 * Specialized Item Definition for armor pieces.
 * Contains armor-specific properties like type, defense values,
 * and resistance modifiers.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAArmorSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAAArmorSettings();

	/* -------------------------------------------------------------------
	 * Armor Properties
	 * ------------------------------------------------------------------- */

	/** The type of armor (Light, Medium, Heavy, Mage, Dragon). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor")
	EAAAArmorType ArmorType = EAAAArmorType::None;

	/** Base physical defense value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor", meta = (ClampMin = "0.0"))
	float PhysicalDefense = 10.0f;

	/** Base magical defense value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor", meta = (ClampMin = "0.0"))
	float MagicalDefense = 5.0f;

	/** Movement speed modifier (0.0 = no penalty, negative = slower). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor")
	float MovementSpeedModifier = 0.0f;

	/** Fire resistance bonus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor|Resistances")
	float FireResistance = 0.0f;

	/** Ice resistance bonus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor|Resistances")
	float IceResistance = 0.0f;

	/** Poison resistance bonus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Armor|Resistances")
	float PoisonResistance = 0.0f;
};
