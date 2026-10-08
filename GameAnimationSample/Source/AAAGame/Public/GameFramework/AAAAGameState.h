// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AAAAGameState.generated.h"

/**
 * AAAAGameState
 * 
 * The global world state manager. 
 * Replicated to all clients to ensure everyone sees the same world events.
 * 
 * Responsibilities:
 * - Current Act / Chapter
 * - Kingdom / Faction Status
 * - Weather & Day/Night Cycle State
 * - Active World Events (Boss Spawns, Festivals, Wars)
 */
UCLASS()
class AAAGAME_API AAAAAGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAAAAGameState();

	//-------------------------------------------------------------------------
	// Future Expansion Points
	//-------------------------------------------------------------------------
	
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AAA|WorldState")
	// int32 CurrentChapter;
	
	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "AAA|WorldState")
	// float TimeOfDay;
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|WorldState")
	// void TriggerWorldEvent(FName EventName);
};
