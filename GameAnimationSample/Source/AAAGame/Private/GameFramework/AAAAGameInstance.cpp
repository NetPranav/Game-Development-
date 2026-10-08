// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "GameFramework/AAAAGameInstance.h"

UAAAAGameInstance::UAAAAGameInstance()
{
	// Default constructor
}

void UAAAAGameInstance::Init()
{
	Super::Init();

	// Initialize Save System, Session Interfaces, Audio, etc.
}

void UAAAAGameInstance::Shutdown()
{
	// Cleanup Session Interfaces, flush saves, etc.
	
	Super::Shutdown();
}
