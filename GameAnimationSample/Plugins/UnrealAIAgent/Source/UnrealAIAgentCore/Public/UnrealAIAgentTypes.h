// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "UnrealAIAgentTypes.generated.h"

/**
 * Safety level for registered AI tools to ensure zero arbitrary code execution.
 */
UENUM(BlueprintType)
enum class EUnrealAIToolSafety : uint8
{
	ReadOnly UMETA(DisplayName = "Read Only", ToolTip = "Safe inspection and query operations with no side-effects."),
	SafeWrite UMETA(DisplayName = "Safe Write", ToolTip = "Operations that modify assets/graphs with undoable transactions."),
	Destructive UMETA(DisplayName = "Destructive", ToolTip = "Operations that delete assets or clear contents, requiring explicit caution.")
};

/**
 * Current operational status of the Unreal AI Agent.
 */
UENUM(BlueprintType)
enum class EUnrealAIAgentStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Planning UMETA(DisplayName = "Planning"),
	ExecutingTool UMETA(DisplayName = "Executing Tool"),
	Compiling UMETA(DisplayName = "Compiling Blueprint"),
	Repairing UMETA(DisplayName = "Repairing Compile Errors"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

/**
 * Represents a single tool call requested by the LLM (NVIDIA NIM).
 */
USTRUCT(BlueprintType)
struct UNREALAIAGENTCORE_API FAIToolCall
{
	GENERATED_BODY()

	/** Unique tool call identifier provided by NIM / OpenAI format */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Id;

	/** Name of the tool to execute */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Name;

	/** Arguments formatted as JSON string */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ArgumentsJson;

	/** Parsed JSON Object for internal processing */
	TSharedPtr<FJsonObject> ParsedArgs;

	FAIToolCall()
		: Id(TEXT(""))
		, Name(TEXT(""))
		, ArgumentsJson(TEXT("{}"))
	{
	}
};

/**
 * Represents the execution result of a tool.
 */
USTRUCT(BlueprintType)
struct UNREALAIAGENTCORE_API FAIToolResult
{
	GENERATED_BODY()

	/** Did the tool execute successfully? */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	bool bSuccess;

	/** Name of the executed tool */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ToolName;

	/** Tool Call ID matching the incoming request */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ToolCallId;

	/** Result payload formatted as JSON string */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ReturnJson;

	/** Error type if execution failed */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ErrorType;

	/** Detailed error message if failed */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ErrorMessage;

	FAIToolResult()
		: bSuccess(false)
		, ToolName(TEXT(""))
		, ToolCallId(TEXT(""))
		, ReturnJson(TEXT("{}"))
		, ErrorType(TEXT(""))
		, ErrorMessage(TEXT(""))
	{
	}

	static FAIToolResult Success(const FString& InToolName, const FString& InToolCallId, const TSharedPtr<FJsonObject>& InResultJson);
	static FAIToolResult Failure(const FString& InToolName, const FString& InToolCallId, const FString& InErrorType, const FString& InErrorMessage);
};

/**
 * Represents a chat message in the conversation context.
 */
USTRUCT(BlueprintType)
struct UNREALAIAGENTCORE_API FAIChatMessage
{
	GENERATED_BODY()

	/** Role: system, user, assistant, tool */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Role;

	/** Text content of the message */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Content;

	/** Tool call ID (for role = tool) */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString ToolCallId;

	/** Name of the tool or sender */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Name;

	/** Tool calls requested by assistant (for role = assistant) */
	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	TArray<FAIToolCall> ToolCalls;

	FAIChatMessage()
		: Role(TEXT("user"))
		, Content(TEXT(""))
		, ToolCallId(TEXT(""))
		, Name(TEXT(""))
	{
	}

	static FAIChatMessage MakeSystem(const FString& InContent)
	{
		FAIChatMessage Msg;
		Msg.Role = TEXT("system");
		Msg.Content = InContent;
		return Msg;
	}

	static FAIChatMessage MakeUser(const FString& InContent)
	{
		FAIChatMessage Msg;
		Msg.Role = TEXT("user");
		Msg.Content = InContent;
		return Msg;
	}

	static FAIChatMessage MakeAssistant(const FString& InContent, const TArray<FAIToolCall>& InToolCalls = TArray<FAIToolCall>())
	{
		FAIChatMessage Msg;
		Msg.Role = TEXT("assistant");
		Msg.Content = InContent;
		Msg.ToolCalls = InToolCalls;
		return Msg;
	}

	static FAIChatMessage MakeTool(const FString& InToolCallId, const FString& InToolName, const FString& InContent)
	{
		FAIChatMessage Msg;
		Msg.Role = TEXT("tool");
		Msg.ToolCallId = InToolCallId;
		Msg.Name = InToolName;
		Msg.Content = InContent;
		return Msg;
	}
};

/**
 * Diagnostic info about Blueprint compile errors.
 */
USTRUCT(BlueprintType)
struct UNREALAIAGENTCORE_API FBlueprintCompileErrorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString BlueprintPath;

	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	FString NodeGuid;

	UPROPERTY(BlueprintReadWrite, Category = "AI Agent")
	bool bIsWarning;

	FBlueprintCompileErrorInfo()
		: BlueprintPath(TEXT(""))
		, Message(TEXT(""))
		, NodeGuid(TEXT(""))
		, bIsWarning(false)
	{
	}
};
