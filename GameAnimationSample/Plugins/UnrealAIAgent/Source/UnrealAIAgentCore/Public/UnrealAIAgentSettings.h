// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UnrealAIAgentSettings.generated.h"

/**
 * Project and Editor Settings for Unreal AI Agent (NVIDIA NIM integration).
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Unreal AI Agent"))
class UNREALAIAGENTCORE_API UUnrealAIAgentSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUnrealAIAgentSettings();

	/**
	 * NVIDIA NIM API Key.
	 * If left empty, the plugin will attempt to read the NVIDIA_NIM_API_KEY environment variable.
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (PasswordField = true, ToolTip = "NVIDIA NIM API Key. Falls back to NVIDIA_NIM_API_KEY environment variable if blank."))
	FString NIMApiKey;

	/**
	 * NVIDIA NIM or OpenAI-compatible endpoint URL for Chat Completions.
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ToolTip = "Chat completions endpoint URL."))
	FString NIMEndpoint;

	/**
	 * Model Name to use on the NIM endpoint.
	 * Examples:
	 *  - nvidia/nemotron-3-ultra-550b-a55b
	 *  - meta/llama-3.3-70b-instruct
	 *  - mistralai/mistral-large-2-instruct
	 *  - deepseek-ai/deepseek-r1
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ToolTip = "Exact NIM model identifier."))
	FString ModelName;

	/**
	 * Sampling temperature for the model (0.0 to 2.0).
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ClampMin = "0.0", ClampMax = "2.0", ToolTip = "Sampling temperature."))
	float Temperature;

	/**
	 * Nucleus sampling probability (Top-P).
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Top-P sampling probability."))
	float TopP;

	/**
	 * Maximum output tokens to generate per request.
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ClampMin = "128", ClampMax = "32768", ToolTip = "Maximum tokens to generate."))
	int32 MaxTokens;

	/**
	 * Enable reasoning/thinking mode (chat_template_kwargs.enable_thinking).
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ToolTip = "Enables thinking/reasoning generation on supported NIM models like Nemotron."))
	bool bEnableThinking;

	/**
	 * Maximum number of agent thought/tool execution steps before forcing termination.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Agent Execution", meta = (ClampMin = "1", ClampMax = "100", ToolTip = "Maximum reasoning / tool steps."))
	int32 MaxSteps;

	/**
	 * Maximum number of automatic compile error repair attempts.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Agent Execution", meta = (ClampMin = "0", ClampMax = "10", ToolTip = "Max self-correction iterations on Blueprint compile errors."))
	int32 MaxRepairAttempts;

	/**
	 * HTTP Request timeout in seconds.
	 */
	UPROPERTY(config, EditAnywhere, Category = "NVIDIA NIM Configuration", meta = (ClampMin = "5.0", ClampMax = "600.0", ToolTip = "HTTP request timeout in seconds."))
	float RequestTimeout;

	/**
	 * Enable detailed debug logging to Output Log.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Debugging", meta = (ToolTip = "Logs all JSON payloads and tool executions to log."))
	bool bDebugLogging;

	/**
	 * Wrap entire AI agent tasks in an Unreal Editor Undo transaction.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Editor Integration", meta = (ToolTip = "Creates an editor transaction so Ctrl+Z undoes the entire AI operation."))
	bool bEnableTransactions;

	/**
	 * Optional custom override for the AI Agent system prompt.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Agent Execution", meta = (MultiLine = true, ToolTip = "Override standard system prompt."))
	FString SystemPromptOverride;

	/** Get effectively configured API Key (checks settings then env var) */
	UFUNCTION(BlueprintPure, Category = "Unreal AI Agent")
	FString GetEffectiveApiKey() const;

	/** Helper to get the settings instance */
	static const UUnrealAIAgentSettings* Get();
	static UUnrealAIAgentSettings* GetMutable();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override { return TEXT("Unreal AI Agent"); }
};
