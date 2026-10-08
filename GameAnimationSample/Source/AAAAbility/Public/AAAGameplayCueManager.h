// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "AAAGameplayCueManager.generated.h"

/**
 * UAAAGameplayCueManager
 *
 * Custom GameplayCue Manager with object pooling enabled.
 *
 * *** GAMEPLAY CUE PATTERN ***
 * GameplayCues handle all cosmetic effects (VFX, SFX, camera shakes)
 * triggered by the ability system. Instead of spawning Niagara/Sound
 * directly from abilities or damage code, you fire a GameplayCue tag:
 *
 *   ASC->ExecuteGameplayCue(GameplayCue.Combat.Hit.Sword, Context);
 *
 * The engine automatically:
 * 1. Finds the matching GameplayCueNotify asset
 * 2. Spawns/recycles the VFX actor from a pool
 * 3. Handles replication (cosmetics only replicate as cue events)
 * 4. Returns the actor to the pool when done
 *
 * Benefits:
 * - Zero cosmetic replication bandwidth (only the tag is sent)
 * - Automatic object pooling (no spawning/destroying actors per hit)
 * - Designers create/modify VFX entirely in Blueprints
 * - Abilities never directly reference Niagara/Sound assets
 *
 * Setup:
 * 1. Create GameplayCueNotify Blueprints in Content/AAA/Effects/GameplayCues/
 * 2. Each Blueprint handles a specific GameplayCue.* tag
 * 3. This manager is configured in DefaultGame.ini:
 *    [/Script/GameplayAbilities.AbilitySystemGlobals]
 *    GameplayCueManagerClassName=/Script/AAAAbility.AAAGameplayCueManager
 */
UCLASS()
class AAAABILITY_API UAAAGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	/** Returns true — we want GameplayCue actors to be recycled from a pool. */
	virtual bool ShouldAsyncLoadObjectLibrariesAtStart() const override { return true; }
};
