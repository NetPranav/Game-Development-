// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "NIM/UnrealNIMClient.h"
#include "UnrealAIAgentSettings.h"
#include "UnrealAIAgentCoreModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FUnrealNIMClient::FUnrealNIMClient()
	: CurrentRetryCount(0)
	, MaxRetryAttempts(3)
{
}

FUnrealNIMClient::~FUnrealNIMClient()
{
	CancelActiveRequest();
}

bool FUnrealNIMClient::SendChatCompletion(
	const TArray<FAIChatMessage>& InMessages,
	const TArray<TSharedPtr<FJsonValue>>& InTools,
	FOnNIMResponseReceived InOnResponse,
	FOnNIMError InOnError)
{
	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	if (!Settings)
	{
		InOnError.ExecuteIfBound(TEXT("Failed to load Unreal AI Agent Settings."));
		return false;
	}

	const FString ApiKey = Settings->GetEffectiveApiKey();
	if (ApiKey.IsEmpty())
	{
		InOnError.ExecuteIfBound(TEXT("NVIDIA NIM API Key is missing. Please configure it in Project Settings -> Unreal AI Agent or set NVIDIA_NIM_API_KEY environment variable."));
		return false;
	}

	CancelActiveRequest();

	PendingResponseDelegate = InOnResponse;
	PendingErrorDelegate = InOnError;
	CurrentRetryCount = 0;

	// Build JSON Payload
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("model"), Settings->ModelName);
	RootObject->SetNumberField(TEXT("temperature"), Settings->Temperature);
	RootObject->SetNumberField(TEXT("top_p"), Settings->TopP);
	RootObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);

	if (Settings->bEnableThinking)
	{
		TSharedPtr<FJsonObject> ChatTemplateKwargs = MakeShared<FJsonObject>();
		ChatTemplateKwargs->SetBoolField(TEXT("enable_thinking"), true);
		RootObject->SetObjectField(TEXT("chat_template_kwargs"), ChatTemplateKwargs);
	}

	// Messages
	TArray<TSharedPtr<FJsonValue>> MessagesJsonArray;
	for (const FAIChatMessage& Msg : InMessages)
	{
		TSharedPtr<FJsonObject> MsgObj = MakeShared<FJsonObject>();
		MsgObj->SetStringField(TEXT("role"), Msg.Role);

		if (!Msg.Content.IsEmpty())
		{
			MsgObj->SetStringField(TEXT("content"), Msg.Content);
		}
		else
		{
			MsgObj->SetStringField(TEXT("content"), TEXT(""));
		}

		if (Msg.Role == TEXT("tool"))
		{
			if (!Msg.ToolCallId.IsEmpty())
			{
				MsgObj->SetStringField(TEXT("tool_call_id"), Msg.ToolCallId);
			}
			if (!Msg.Name.IsEmpty())
			{
				MsgObj->SetStringField(TEXT("name"), Msg.Name);
			}
		}

		if (Msg.Role == TEXT("assistant") && Msg.ToolCalls.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> ToolCallsArray;
			for (const FAIToolCall& TC : Msg.ToolCalls)
			{
				TSharedPtr<FJsonObject> ToolCallObj = MakeShared<FJsonObject>();
				ToolCallObj->SetStringField(TEXT("id"), TC.Id);
				ToolCallObj->SetStringField(TEXT("type"), TEXT("function"));

				TSharedPtr<FJsonObject> FuncObj = MakeShared<FJsonObject>();
				FuncObj->SetStringField(TEXT("name"), TC.Name);
				FuncObj->SetStringField(TEXT("arguments"), TC.ArgumentsJson);
				ToolCallObj->SetObjectField(TEXT("function"), FuncObj);

				ToolCallsArray.Add(MakeShared<FJsonValueObject>(ToolCallObj));
			}
			MsgObj->SetArrayField(TEXT("tool_calls"), ToolCallsArray);
		}

		MessagesJsonArray.Add(MakeShared<FJsonValueObject>(MsgObj));
	}
	RootObject->SetArrayField(TEXT("messages"), MessagesJsonArray);

	// Tools
	if (InTools.Num() > 0)
	{
		RootObject->SetArrayField(TEXT("tools"), InTools);
		RootObject->SetStringField(TEXT("tool_choice"), TEXT("auto"));
	}

	CachedPayloadString.Empty();
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&CachedPayloadString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	if (Settings->bDebugLogging)
	{
		UE_LOG(LogUnrealAIAgentCore, Log, TEXT("Sending request to NIM Endpoint: %s with %d messages, %d tools"),
			*Settings->NIMEndpoint, InMessages.Num(), InTools.Num());
	}

	SendPayloadInternal();
	return true;
}

void FUnrealNIMClient::SendPayloadInternal()
{
	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	const FString ApiKey = Settings ? Settings->GetEffectiveApiKey() : TEXT("");
	const float TimeoutSeconds = Settings ? FMath::Max(600.0f, Settings->RequestTimeout) : 600.0f;

	ActiveHttpRequest = FHttpModule::Get().CreateRequest();
	ActiveHttpRequest->SetURL(Settings ? Settings->NIMEndpoint : TEXT("https://integrate.api.nvidia.com/v1/chat/completions"));
	ActiveHttpRequest->SetVerb(TEXT("POST"));
	ActiveHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	ActiveHttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
	ActiveHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	ActiveHttpRequest->SetTimeout(TimeoutSeconds);
	ActiveHttpRequest->SetContentAsString(CachedPayloadString);

	ActiveHttpRequest->OnProcessRequestComplete().BindSP(this, &FUnrealNIMClient::OnHttpRequestComplete);

	if (!ActiveHttpRequest->ProcessRequest())
	{
		ActiveHttpRequest.Reset();
		PendingErrorDelegate.ExecuteIfBound(TEXT("Failed to initialize and dispatch HTTP request to NVIDIA NIM."));
	}
}

void FUnrealNIMClient::CancelActiveRequest()
{
	if (ActiveHttpRequest.IsValid())
	{
		ActiveHttpRequest->CancelRequest();
		ActiveHttpRequest.Reset();
	}
}

bool FUnrealNIMClient::IsRequestActive() const
{
	return ActiveHttpRequest.IsValid() && ActiveHttpRequest->GetStatus() == EHttpRequestStatus::Processing;
}

void FUnrealNIMClient::OnHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	ActiveHttpRequest.Reset();

	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		if (CurrentRetryCount < MaxRetryAttempts)
		{
			CurrentRetryCount++;
			UE_LOG(LogUnrealAIAgentCore, Warning, TEXT("NIM request connection dropped, auto-retrying attempt %d/%d..."), CurrentRetryCount, MaxRetryAttempts);
			SendPayloadInternal();
			return;
		}

		FString ErrMsg = TEXT("Network connection failed. Could not reach NVIDIA NIM endpoint.");
		UE_LOG(LogUnrealAIAgentCore, Error, TEXT("%s"), *ErrMsg);
		PendingErrorDelegate.ExecuteIfBound(ErrMsg);
		return;
	}

	const int32 StatusCode = Response->GetResponseCode();
	const FString ResponseString = Response->GetContentAsString();

	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	if (Settings && Settings->bDebugLogging)
	{
		UE_LOG(LogUnrealAIAgentCore, Log, TEXT("NIM Response (%d): %s"), StatusCode, *ResponseString);
	}

	if (StatusCode != 200)
	{
		if ((StatusCode >= 500 || StatusCode == 429) && CurrentRetryCount < MaxRetryAttempts)
		{
			CurrentRetryCount++;
			UE_LOG(LogUnrealAIAgentCore, Warning, TEXT("NIM returned HTTP %d, auto-retrying attempt %d/%d..."), StatusCode, CurrentRetryCount, MaxRetryAttempts);
			SendPayloadInternal();
			return;
		}

		FString ErrorDetail = FString::Printf(TEXT("NVIDIA NIM Error (HTTP %d): "), StatusCode);

		// Try to parse detailed error json
		TSharedPtr<FJsonObject> JsonObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
		if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
		{
			if (JsonObj->HasTypedField<EJson::Object>(TEXT("error")))
			{
				TSharedPtr<FJsonObject> ErrObj = JsonObj->GetObjectField(TEXT("error"));
				if (ErrObj->HasField(TEXT("message")))
				{
					ErrorDetail += ErrObj->GetStringField(TEXT("message"));
				}
				else
				{
					ErrorDetail += ResponseString;
				}
			}
			else if (JsonObj->HasField(TEXT("detail")))
			{
				ErrorDetail += JsonObj->GetStringField(TEXT("detail"));
			}
			else
			{
				ErrorDetail += ResponseString;
			}
		}
		else
		{
			ErrorDetail += ResponseString;
		}

		UE_LOG(LogUnrealAIAgentCore, Error, TEXT("%s"), *ErrorDetail);
		PendingErrorDelegate.ExecuteIfBound(ErrorDetail);
		return;
	}

	FString AssistantContent;
	FString ReasoningContent;
	TArray<FAIToolCall> ToolCalls;
	FString ParseError;

	if (!ParseNIMResponse(ResponseString, AssistantContent, ReasoningContent, ToolCalls, ParseError))
	{
		UE_LOG(LogUnrealAIAgentCore, Error, TEXT("Failed to parse NIM response: %s"), *ParseError);
		PendingErrorDelegate.ExecuteIfBound(ParseError);
		return;
	}

	PendingResponseDelegate.ExecuteIfBound(AssistantContent, ReasoningContent, ToolCalls, true);
}

bool FUnrealNIMClient::ParseNIMResponse(
	const FString& ResponseString,
	FString& OutContent,
	FString& OutReasoning,
	TArray<FAIToolCall>& OutToolCalls,
	FString& OutError)
{
	TSharedPtr<FJsonObject> RootObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		OutError = TEXT("Failed to parse response body as valid JSON.");
		return false;
	}

	if (!RootObj->HasTypedField<EJson::Array>(TEXT("choices")))
	{
		OutError = TEXT("Response JSON missing 'choices' array.");
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>& Choices = RootObj->GetArrayField(TEXT("choices"));
	if (Choices.Num() == 0 || !Choices[0]->AsObject().IsValid())
	{
		OutError = TEXT("'choices' array is empty in NIM response.");
		return false;
	}

	TSharedPtr<FJsonObject> FirstChoice = Choices[0]->AsObject();
	if (!FirstChoice->HasTypedField<EJson::Object>(TEXT("message")))
	{
		OutError = TEXT("First choice missing 'message' object.");
		return false;
	}

	TSharedPtr<FJsonObject> MessageObj = FirstChoice->GetObjectField(TEXT("message"));

	if (MessageObj->HasTypedField<EJson::String>(TEXT("content")))
	{
		OutContent = MessageObj->GetStringField(TEXT("content"));
	}
	else
	{
		OutContent.Empty();
	}

	if (MessageObj->HasTypedField<EJson::String>(TEXT("reasoning_content")))
	{
		OutReasoning = MessageObj->GetStringField(TEXT("reasoning_content"));
	}
	else if (MessageObj->HasTypedField<EJson::String>(TEXT("reasoning")))
	{
		OutReasoning = MessageObj->GetStringField(TEXT("reasoning"));
	}
	else if (FirstChoice->HasTypedField<EJson::String>(TEXT("reasoning_content")))
	{
		OutReasoning = FirstChoice->GetStringField(TEXT("reasoning_content"));
	}
	else if (FirstChoice->HasTypedField<EJson::String>(TEXT("reasoning")))
	{
		OutReasoning = FirstChoice->GetStringField(TEXT("reasoning"));
	}
	else
	{
		OutReasoning.Empty();
	}

	// Parse tool calls if any
	OutToolCalls.Empty();
	if (MessageObj->HasTypedField<EJson::Array>(TEXT("tool_calls")))
	{
		const TArray<TSharedPtr<FJsonValue>>& ToolCallsJson = MessageObj->GetArrayField(TEXT("tool_calls"));
		for (int32 Idx = 0; Idx < ToolCallsJson.Num(); ++Idx)
		{
			TSharedPtr<FJsonObject> TCItem = ToolCallsJson[Idx]->AsObject();
			if (!TCItem.IsValid())
			{
				continue;
			}

			FAIToolCall ToolCall;
			ToolCall.Id = TCItem->GetStringField(TEXT("id"));
			if (ToolCall.Id.IsEmpty())
			{
				ToolCall.Id = FString::Printf(TEXT("call_%d"), Idx);
			}

			if (TCItem->HasTypedField<EJson::Object>(TEXT("function")))
			{
				TSharedPtr<FJsonObject> FuncObj = TCItem->GetObjectField(TEXT("function"));
				ToolCall.Name = FuncObj->GetStringField(TEXT("name"));

				// arguments may be a string or an embedded object
				if (FuncObj->HasTypedField<EJson::String>(TEXT("arguments")))
				{
					ToolCall.ArgumentsJson = FuncObj->GetStringField(TEXT("arguments"));
				}
				else if (FuncObj->HasTypedField<EJson::Object>(TEXT("arguments")))
				{
					TSharedPtr<FJsonObject> ArgsObj = FuncObj->GetObjectField(TEXT("arguments"));
					FString SerializedArgs;
					TSharedRef<TJsonWriter<>> ArgWriter = TJsonWriterFactory<>::Create(&SerializedArgs);
					FJsonSerializer::Serialize(ArgsObj.ToSharedRef(), ArgWriter);
					ToolCall.ArgumentsJson = SerializedArgs;
					ToolCall.ParsedArgs = ArgsObj;
				}
				else
				{
					ToolCall.ArgumentsJson = TEXT("{}");
				}
			}

			OutToolCalls.Add(ToolCall);
		}
	}

	return true;
}
