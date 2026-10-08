// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UnrealAIAgentEditorModule.h"
#include "UnrealAIAgentStyle.h"
#include "UnrealAIAgentCommands.h"
#include "UI/SUnrealAIAgentWindow.h"
#include "Registry/UnrealAIToolRegistry.h"
#include "Tools/AssetTools/UnrealAIAssetTools.h"
#include "Tools/BlueprintTools/UnrealAIBlueprintTools.h"
#include "Tools/BlueprintTools/UnrealAIBlueprintGraphTools.h"
#include "Tools/BlueprintTools/UnrealAIBlueprintNodeTools.h"
#include "Tools/BlueprintTools/UnrealAIBlueprintComponentTools.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

DEFINE_LOG_CATEGORY(LogUnrealAIAgentEditor);

static const FName UnrealAIAgentTabName("UnrealAIAgentWindow");

#define LOCTEXT_NAMESPACE "FUnrealAIAgentEditorModule"

void FUnrealAIAgentEditorModule::StartupModule()
{
	UE_LOG(LogUnrealAIAgentEditor, Log, TEXT("UnrealAIAgentEditor module starting up..."));

	FUnrealAIAgentStyle::Initialize();
	FUnrealAIAgentStyle::ReloadTextures();

	FUnrealAIAgentCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FUnrealAIAgentCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUnrealAIAgentEditorModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUnrealAIAgentEditorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		UnrealAIAgentTabName,
		FOnSpawnTab::CreateRaw(this, &FUnrealAIAgentEditorModule::OnSpawnPluginTab)
	)
	.SetDisplayName(LOCTEXT("FUnrealAIAgentTabTitle", "Antigravity AI"))
	.SetTooltipText(LOCTEXT("FUnrealAIAgentTabTooltip", "Open the Antigravity AI right-sidebar agent panel."))
	.SetIcon(FSlateIcon(FUnrealAIAgentStyle::GetStyleSetName(), "UnrealAIAgent.OpenPluginWindow"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());

	RegisterAllTools();
}

void FUnrealAIAgentEditorModule::ShutdownModule()
{
	UE_LOG(LogUnrealAIAgentEditor, Log, TEXT("UnrealAIAgentEditor module shutting down..."));

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FUnrealAIAgentStyle::Shutdown();
	FUnrealAIAgentCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UnrealAIAgentTabName);
}

TSharedRef<SDockTab> FUnrealAIAgentEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("FUnrealAIAgentTabTitle", "Antigravity AI"))
		[
			SNew(SUnrealAIAgentWindow)
		];
}

void FUnrealAIAgentEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UnrealAIAgentTabName);
}

void FUnrealAIAgentEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUnrealAIAgentCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUnrealAIAgentCommands::Get().OpenPluginWindow));
			Entry.SetCommandList(PluginCommands);
		}
	}
}

void FUnrealAIAgentEditorModule::RegisterAllTools()
{
	FUnrealAIToolRegistry& Registry = FUnrealAIToolRegistry::Get();

	// Asset Tools
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetProjectInfo>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_FindAsset>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetAssetInfo>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_CreateAssetFolder>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_MoveAsset>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_RenameAsset>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_DeleteAsset>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_SaveAsset>());

	// Blueprint Asset Tools
	Registry.RegisterTool(MakeShared<FUnrealAITool_CreateBlueprint>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_OpenBlueprint>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_InspectBlueprint>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_SaveBlueprint>());

	// Blueprint Graph & Variable Tools
	Registry.RegisterTool(MakeShared<FUnrealAITool_ListBlueprintGraphs>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_CreateVariable>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_ModifyVariable>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_DeleteVariable>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_CreateFunction>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_DeleteFunction>());

	// Blueprint Graph Node & Pin Tools
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetGraphNodes>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetNodeInfo>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetNodePins>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_FindNode>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_FindPin>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_CreateNode>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_DeleteNode>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_MoveNode>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_SetNodeProperty>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_ConnectPins>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_DisconnectPins>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_CompileBlueprint>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_GetCompileResults>());

	// Blueprint Component Tools
	Registry.RegisterTool(MakeShared<FUnrealAITool_AddComponent>());
	Registry.RegisterTool(MakeShared<FUnrealAITool_SetComponentProperty>());

	UE_LOG(LogUnrealAIAgentEditor, Log, TEXT("Unreal AI Agent initialized %d tools into central registry."), Registry.GetAllTools().Num());
}

FUnrealAIAgentEditorModule& FUnrealAIAgentEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FUnrealAIAgentEditorModule>("UnrealAIAgentEditor");
}

bool FUnrealAIAgentEditorModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("UnrealAIAgentEditor");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealAIAgentEditorModule, UnrealAIAgentEditor)
