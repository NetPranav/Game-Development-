// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealAIAgentCore, Log, All);

class FUnrealAIAgentCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get singleton instance of Core module */
	static FUnrealAIAgentCoreModule& Get();

	/** Check if module is loaded */
	static bool IsAvailable();
};
