// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FUnrealAIAgentCommands : public TCommands<FUnrealAIAgentCommands>
{
public:
	FUnrealAIAgentCommands()
		: TCommands<FUnrealAIAgentCommands>(
			TEXT("UnrealAIAgent"),
			NSLOCTEXT("Contexts", "UnrealAIAgent", "Unreal AI Agent Plugin"),
			NAME_None,
			FAppStyle::GetAppStyleSetName()
		)
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
