// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "GameFramework/AAAAGameModeBase.h"
#include "UObject/ConstructorHelpers.h"

// Framework Includes
#include "GameFramework/AAAAPlayerController.h"
#include "GameFramework/AAAAPlayerState.h"
#include "GameFramework/AAAAGameState.h"
#include "GameFramework/AAAAHUD.h"

AAAAAGameModeBase::AAAAAGameModeBase()
{
	// 1. Assign Game State
	GameStateClass = AAAAAGameState::StaticClass();

	// 2. Assign Player Controller
	PlayerControllerClass = AAAAAPlayerController::StaticClass();

	// 3. Assign Player State
	PlayerStateClass = AAAAAPlayerState::StaticClass();

	// 4. Assign HUD
	HUDClass = AAAAAHUD::StaticClass();

	// 5. Assign Default Pawn (Player Character)
	// (Left empty in C++ so it relies entirely on BP_AAAGameMode in the editor)
}
