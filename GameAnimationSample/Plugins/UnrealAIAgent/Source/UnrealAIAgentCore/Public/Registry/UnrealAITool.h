// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "UnrealAIAgentTypes.h"

/**
 * Interface representing an executable, structured AI tool inside Unreal Editor.
 */
class UNREALAIAGENTCORE_API IUnrealAITool
{
public:
	virtual ~IUnrealAITool() = default;

	/** Unique name of the tool (snake_case) */
	virtual FString GetName() const = 0;

	/** Human and LLM readable description of what the tool does */
	virtual FString GetDescription() const = 0;

	/** JSON Schema object describing the required and optional input parameters */
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const = 0;

	/** Safety classification (ReadOnly, SafeWrite, Destructive) */
	virtual EUnrealAIToolSafety GetSafetyLevel() const = 0;

	/**
	 * Validate input parameters before execution.
	 * Returns true if valid, or false with an error message.
	 */
	virtual bool ValidateParameters(const TSharedPtr<FJsonObject>& InArgs, FString& OutErrorMessage) const;

	/**
	 * Execute the tool with validated arguments.
	 * Must populate OutResult with success/failure and structured JSON return data.
	 */
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) = 0;
};
