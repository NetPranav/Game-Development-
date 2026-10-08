// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UnrealAIAgentCommands.h"

#define LOCTEXT_NAMESPACE "FUnrealAIAgentCommands"

void FUnrealAIAgentCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Unreal AI Agent", "Open the Unreal AI Agent window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
