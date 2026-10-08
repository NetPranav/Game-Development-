// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAAConsumableSettings.generated.h"

class UGameplayEffect;

/**
 * UAAAConsumableSettings
 *
 * Specialized Item Definition for consumable items (potions, food, scrolls).
 * References a Gameplay Effect to apply on use. No application logic here.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAConsumableSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAAConsumableSettings();

	/** The Gameplay Effect to apply when this consumable is used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Consumable")
	TSoftClassPtr<UGameplayEffect> EffectToApply;

	/** Duration in seconds. 0 = Instant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Consumable", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	/** Cooldown in seconds before this consumable type can be used again. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Consumable", meta = (ClampMin = "0.0"))
	float Cooldown = 0.0f;

	/** Whether this item is consumed on use (decrements stack). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Consumable")
	bool bConsumeOnUse = true;
};
