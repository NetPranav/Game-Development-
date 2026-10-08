// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AAADamageExecutionCalc.generated.h"

/**
 * Custom execution calculation for damage.
 * Takes the Base Damage defined by the effect and subtracts the target's Armor.
 */
UCLASS()
class AAAABILITY_API UAAADamageExecutionCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UAAADamageExecutionCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
