// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Docking/SDockTab.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealAIAgentEditor, Log, All);

class FUnrealAIAgentEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Tab Spawner Delegate */
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	/** Trigger opening the agent tab */
	void PluginButtonClicked();

	static FUnrealAIAgentEditorModule& Get();
	static bool IsAvailable();

private:
	void RegisterMenus();
	void RegisterAllTools();

	TSharedPtr<FUICommandList> PluginCommands;
};
