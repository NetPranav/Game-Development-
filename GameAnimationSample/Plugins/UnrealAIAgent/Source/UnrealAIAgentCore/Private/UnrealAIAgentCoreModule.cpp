// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UnrealAIAgentCoreModule.h"
#include "Modules/ModuleManager.h"
#include "UnrealAIAgentSettings.h"
#include "Registry/UnrealAIToolRegistry.h"

DEFINE_LOG_CATEGORY(LogUnrealAIAgentCore);

#define LOCTEXT_NAMESPACE "FUnrealAIAgentCoreModule"

void FUnrealAIAgentCoreModule::StartupModule()
{
	UE_LOG(LogUnrealAIAgentCore, Log, TEXT("UnrealAIAgentCore module starting up..."));
}

void FUnrealAIAgentCoreModule::ShutdownModule()
{
	UE_LOG(LogUnrealAIAgentCore, Log, TEXT("UnrealAIAgentCore module shutting down..."));
	FUnrealAIToolRegistry::Get().ClearAllTools();
}

FUnrealAIAgentCoreModule& FUnrealAIAgentCoreModule::Get()
{
	return FModuleManager::LoadModuleChecked<FUnrealAIAgentCoreModule>("UnrealAIAgentCore");
}

bool FUnrealAIAgentCoreModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("UnrealAIAgentCore");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealAIAgentCoreModule, UnrealAIAgentCore)
