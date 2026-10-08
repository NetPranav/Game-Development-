// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Registry/UnrealAIToolRegistry.h"
#include "UnrealAIAgentCoreModule.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FAIToolResult FAIToolResult::Success(const FString& InToolName, const FString& InToolCallId, const TSharedPtr<FJsonObject>& InResultJson)
{
	FAIToolResult Result;
	Result.bSuccess = true;
	Result.ToolName = InToolName;
	Result.ToolCallId = InToolCallId;

	if (InResultJson.IsValid())
	{
		// Ensure success field is set
		if (!InResultJson->HasField(TEXT("success")))
		{
			InResultJson->SetBoolField(TEXT("success"), true);
		}
		if (!InResultJson->HasField(TEXT("tool")))
		{
			InResultJson->SetStringField(TEXT("tool"), InToolName);
		}

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(InResultJson.ToSharedRef(), Writer);
		Result.ReturnJson = OutputString;
	}
	else
	{
		Result.ReturnJson = FString::Printf(TEXT("{\"success\": true, \"tool\": \"%s\"}"), *InToolName);
	}

	return Result;
}

FAIToolResult FAIToolResult::Failure(const FString& InToolName, const FString& InToolCallId, const FString& InErrorType, const FString& InErrorMessage)
{
	FAIToolResult Result;
	Result.bSuccess = false;
	Result.ToolName = InToolName;
	Result.ToolCallId = InToolCallId;
	Result.ErrorType = InErrorType;
	Result.ErrorMessage = InErrorMessage;

	TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
	JsonObj->SetBoolField(TEXT("success"), false);
	JsonObj->SetStringField(TEXT("tool"), InToolName);
	JsonObj->SetStringField(TEXT("error_type"), InErrorType);
	JsonObj->SetStringField(TEXT("message"), InErrorMessage);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	Result.ReturnJson = OutputString;

	return Result;
}

bool IUnrealAITool::ValidateParameters(const TSharedPtr<FJsonObject>& InArgs, FString& OutErrorMessage) const
{
	if (!InArgs.IsValid())
	{
		OutErrorMessage = TEXT("Arguments object is null or invalid JSON.");
		return false;
	}

	TSharedPtr<FJsonObject> Schema = GetParametersSchema();
	if (!Schema.IsValid())
	{
		return true; // No schema restrictions
	}

	// Check required fields
	if (Schema->HasTypedField<EJson::Array>(TEXT("required")))
	{
		const TArray<TSharedPtr<FJsonValue>>& RequiredFields = Schema->GetArrayField(TEXT("required"));
		for (const TSharedPtr<FJsonValue>& RequiredVal : RequiredFields)
		{
			FString FieldName = RequiredVal->AsString();
			if (!InArgs->HasField(FieldName))
			{
				OutErrorMessage = FString::Printf(TEXT("Missing required parameter '%s' for tool '%s'."), *FieldName, *GetName());
				return false;
			}
		}
	}

	return true;
}

FUnrealAIToolRegistry& FUnrealAIToolRegistry::Get()
{
	static FUnrealAIToolRegistry Instance;
	return Instance;
}

void FUnrealAIToolRegistry::RegisterTool(TSharedRef<IUnrealAITool> InTool)
{
	const FString ToolName = InTool->GetName();
	if (ToolName.IsEmpty())
	{
		UE_LOG(LogUnrealAIAgentCore, Warning, TEXT("Attempted to register tool with empty name."));
		return;
	}

	RegisteredTools.Add(ToolName, InTool);
	UE_LOG(LogUnrealAIAgentCore, Log, TEXT("Registered AI Tool: %s"), *ToolName);
}

void FUnrealAIToolRegistry::UnregisterTool(const FString& InToolName)
{
	RegisteredTools.Remove(InToolName);
}

TSharedPtr<IUnrealAITool> FUnrealAIToolRegistry::FindTool(const FString& InToolName) const
{
	if (const TSharedRef<IUnrealAITool>* Found = RegisteredTools.Find(InToolName))
	{
		return *Found;
	}
	return nullptr;
}

void FUnrealAIToolRegistry::ClearAllTools()
{
	RegisteredTools.Empty();
}

TArray<TSharedPtr<FJsonValue>> FUnrealAIToolRegistry::GenerateNIMToolsJson() const
{
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	for (const auto& Pair : RegisteredTools)
	{
		const TSharedRef<IUnrealAITool>& Tool = Pair.Value;

		TSharedPtr<FJsonObject> FunctionObj = MakeShared<FJsonObject>();
		FunctionObj->SetStringField(TEXT("name"), Tool->GetName());
		FunctionObj->SetStringField(TEXT("description"), Tool->GetDescription());

		TSharedPtr<FJsonObject> Schema = Tool->GetParametersSchema();
		if (Schema.IsValid())
		{
			FunctionObj->SetObjectField(TEXT("parameters"), Schema);
		}
		else
		{
			TSharedPtr<FJsonObject> EmptySchema = MakeShared<FJsonObject>();
			EmptySchema->SetStringField(TEXT("type"), TEXT("object"));
			EmptySchema->SetObjectField(TEXT("properties"), MakeShared<FJsonObject>());
			FunctionObj->SetObjectField(TEXT("parameters"), EmptySchema);
		}

		TSharedPtr<FJsonObject> ToolContainer = MakeShared<FJsonObject>();
		ToolContainer->SetStringField(TEXT("type"), TEXT("function"));
		ToolContainer->SetObjectField(TEXT("function"), FunctionObj);

		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolContainer));
	}

	return ToolsArray;
}

FAIToolResult FUnrealAIToolRegistry::ExecuteToolCall(const FAIToolCall& InToolCall)
{
	const FString ToolName = InToolCall.Name;
	TSharedPtr<IUnrealAITool> Tool = FindTool(ToolName);

	if (!Tool.IsValid())
	{
		return FAIToolResult::Failure(
			ToolName,
			InToolCall.Id,
			TEXT("TOOL_NOT_FOUND"),
			FString::Printf(TEXT("Tool '%s' is not registered in the Unreal AI Tool Registry."), *ToolName)
		);
	}

	// Parse JSON arguments
	TSharedPtr<FJsonObject> JsonArgs;
	if (InToolCall.ParsedArgs.IsValid())
	{
		JsonArgs = InToolCall.ParsedArgs;
	}
	else if (!InToolCall.ArgumentsJson.IsEmpty())
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InToolCall.ArgumentsJson);
		if (!FJsonSerializer::Deserialize(Reader, JsonArgs) || !JsonArgs.IsValid())
		{
			return FAIToolResult::Failure(
				ToolName,
				InToolCall.Id,
				TEXT("INVALID_ARGUMENT_JSON"),
				FString::Printf(TEXT("Failed to parse arguments JSON for tool '%s': %s"), *ToolName, *InToolCall.ArgumentsJson)
			);
		}
	}
	else
	{
		JsonArgs = MakeShared<FJsonObject>();
	}

	// Validate arguments against tool schema
	FString ValidationError;
	if (!Tool->ValidateParameters(JsonArgs, ValidationError))
	{
		return FAIToolResult::Failure(
			ToolName,
			InToolCall.Id,
			TEXT("PARAMETER_VALIDATION_FAILED"),
			ValidationError
		);
	}

	// Execute tool
	FAIToolResult ExecutionResult;
	ExecutionResult.ToolCallId = InToolCall.Id;
	ExecutionResult.ToolName = ToolName;

	try
	{
		const bool bExecSuccess = Tool->Execute(JsonArgs, ExecutionResult);
		ExecutionResult.bSuccess = bExecSuccess;
		ExecutionResult.ToolCallId = InToolCall.Id;
		ExecutionResult.ToolName = ToolName;
	}
	catch (...)
	{
		return FAIToolResult::Failure(
			ToolName,
			InToolCall.Id,
			TEXT("INTERNAL_EXECUTION_EXCEPTION"),
			FString::Printf(TEXT("An unhandled C++ exception occurred while executing tool '%s'."), *ToolName)
		);
	}

	return ExecutionResult;
}
