// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Registry/UnrealAITool.h"
#include "UnrealAIAgentTypes.h"

/**
 * Centralized tool registry managing all registered AI capabilities.
 */
class UNREALAIAGENTCORE_API FUnrealAIToolRegistry
{
public:
	/** Get singleton instance */
	static FUnrealAIToolRegistry& Get();

	/** Register a new tool instance */
	void RegisterTool(TSharedRef<IUnrealAITool> InTool);

	/** Unregister a tool by name */
	void UnregisterTool(const FString& InToolName);

	/** Find tool by name */
	TSharedPtr<IUnrealAITool> FindTool(const FString& InToolName) const;

	/** Get all registered tools */
	const TMap<FString, TSharedRef<IUnrealAITool>>& GetAllTools() const { return RegisteredTools; }

	/** Clear all registered tools */
	void ClearAllTools();

	/**
	 * Formats all registered tools into the standard JSON array expected by NVIDIA NIM / OpenAI tool-calling endpoints.
	 */
	TArray<TSharedPtr<FJsonValue>> GenerateNIMToolsJson() const;

	/**
	 * Validates arguments and executes a tool call, returning a structured FAIToolResult.
	 */
	FAIToolResult ExecuteToolCall(const FAIToolCall& InToolCall);

private:
	FUnrealAIToolRegistry() = default;
	~FUnrealAIToolRegistry() = default;

	TMap<FString, TSharedRef<IUnrealAITool>> RegisteredTools;
};
