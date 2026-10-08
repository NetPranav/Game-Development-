// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AAAAGameModeBase.generated.h"

/**
 * AAAAGameModeBase
 * 
 * The master authority for a game match. Server-only.
 * 
 * Responsibilities:
 * - Match Flow (Pre-match, In-progress, Post-match)
 * - Spawning and Respawning Players
 * - Setting difficulty and game rules
 * - Multiplayer Authority over critical state changes
 * 
 * Note: No combat, UI, or inventory logic should be here.
 */
UCLASS()
class AAAGAME_API AAAAAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAAAAGameModeBase();

	//-------------------------------------------------------------------------
	// Future Expansion Points
	//-------------------------------------------------------------------------
	
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	// virtual void Logout(AController* Exiting) override;
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|Respawn")
	// void RequestRespawn(AController* PlayerController);
};
