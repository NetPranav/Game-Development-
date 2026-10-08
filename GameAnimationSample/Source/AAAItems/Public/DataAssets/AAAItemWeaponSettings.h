// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAAItemTypes.h"
#include "AAAItemWeaponSettings.generated.h"

/**
 * UAAAItemWeaponSettings
 *
 * Specialized Item Definition for weapons.
 * (Named ItemWeaponSettings to avoid collision with AAACombat's AAAWeaponSettings).
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAItemWeaponSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAAItemWeaponSettings();

	/* -------------------------------------------------------------------
	 * Weapon Properties
	 * ------------------------------------------------------------------- */

	/** The type of weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Weapon")
	EAAAItemCategory WeaponCategory = EAAAItemCategory::Weapon; // Avoid enum collision

	/** Whether this weapon requires two hands (blocks offhand slot). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Weapon")
	bool bIsTwoHanded = false;

	/** Base minimum damage value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Weapon", meta = (ClampMin = "0.0"))
	float BaseDamageMin = 10.0f;

	/** Base maximum damage value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Weapon", meta = (ClampMin = "0.0"))
	float BaseDamageMax = 20.0f;
};
