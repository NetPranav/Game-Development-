// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AAAAGameInstance.generated.h"

/**
 * AAAAGameInstance
 * 
 * The highest-level manager in the game. It persists throughout the entire application lifetime,
 * surviving map travels and session changes.
 * 
 * Responsibilities:
 * - Save / Load System
 * - Session Management (EOS, Steam)
 * - Audio Settings
 * - Graphics Settings
 * - Persistent Runtime Data (Character Selection)
 * - Map Travel
 */
UCLASS()
class AAAGAME_API UAAAAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAAAAGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	//-------------------------------------------------------------------------
	// Future Expansion Points
	//-------------------------------------------------------------------------
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|Save")
	// void SaveGame();
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|Save")
	// void LoadGame();
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|Session")
	// void HostSession();
	
	// UFUNCTION(BlueprintCallable, Category = "AAA|Session")
	// void JoinSession();
};
