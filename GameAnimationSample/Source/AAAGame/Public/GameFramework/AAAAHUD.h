// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AAAAHUD.generated.h"

/**
 * AAAAHUD
 * 
 * The client-side UI manager. 
 * 
 * Responsibilities:
 * - Spawning and maintaining references to global widgets (HUD, Inventory, Quests, Compass).
 * - Toggling UI visibility (e.g., hiding HUD during cutscenes).
 * - Drawing debug info if needed.
 * 
 * Note: No gameplay logic should exist here.
 */
UCLASS()
class AAAGAME_API AAAAAHUD : public AHUD
{
	GENERATED_BODY()

public:
	AAAAAHUD();

	//-------------------------------------------------------------------------
	// UI Management
	//-------------------------------------------------------------------------
	
	/** The Blueprint class to spawn for the Game Menu (Inventory/Map/Scratchpad) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|UI")
	TSubclassOf<class UUserWidget> GameMenuClass;
	
	/** The instantiated Game Menu widget */
	UPROPERTY(BlueprintReadOnly, Category = "AAA|UI")
	TObjectPtr<class UUserWidget> GameMenuWidget;
	
	/** Toggles the Game Menu open and closed. Returns true if the menu is now open. */
	UFUNCTION(BlueprintCallable, Category = "AAA|UI")
	virtual bool ToggleGameMenu();

protected:
	virtual void BeginPlay() override;

public:
	virtual void DrawHUD() override;
};
