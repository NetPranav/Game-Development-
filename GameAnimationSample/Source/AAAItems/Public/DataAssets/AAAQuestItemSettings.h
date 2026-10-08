// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAItemDefinition.h"
#include "AAAQuestItemSettings.generated.h"

/**
 * UAAAQuestItemSettings
 *
 * Specialized Item Definition for quest items and key items.
 * These items are typically non-stackable, non-sellable, and persist until
 * quest completion.
 */
UCLASS(BlueprintType)
class AAAITEMS_API UAAAQuestItemSettings : public UAAAItemDefinition
{
	GENERATED_BODY()

public:
	UAAAQuestItemSettings();

	/** The quest ID this item is associated with. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Quest")
	FName AssociatedQuestID;

	/** Whether this item should be auto-removed when the quest completes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Quest")
	bool bRemoveOnQuestComplete = true;

	/** Whether the player can voluntarily drop this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Item|Quest")
	bool bCanDrop = false;
};
