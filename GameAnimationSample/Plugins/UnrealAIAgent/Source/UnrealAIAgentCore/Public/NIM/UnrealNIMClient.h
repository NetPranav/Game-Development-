// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Interfaces/IHttpRequest.h"
#include "UnrealAIAgentTypes.h"

DECLARE_DELEGATE_FourParams(FOnNIMResponseReceived, const FString& /*AssistantReply*/, const FString& /*ReasoningContent*/, const TArray<FAIToolCall>& /*ToolCalls*/, bool /*bSuccess*/);
DECLARE_DELEGATE_OneParam(FOnNIMError, const FString& /*ErrorMessage*/);

/**
 * REST Client communicating with NVIDIA NIM and OpenAI-compatible Chat Completion / Function Calling endpoints.
 */
class UNREALAIAGENTCORE_API FUnrealNIMClient : public TSharedFromThis<FUnrealNIMClient>
{
public:
	FUnrealNIMClient();
	~FUnrealNIMClient();

	/**
	 * Send a chat completion request with message history and available tool specifications.
	 */
	bool SendChatCompletion(
		const TArray<FAIChatMessage>& InMessages,
		const TArray<TSharedPtr<FJsonValue>>& InTools,
		FOnNIMResponseReceived InOnResponse,
		FOnNIMError InOnError
	);

	/** Cancel currently in-flight HTTP request if any */
	void CancelActiveRequest();

	/** Check if a request is currently active */
	bool IsRequestActive() const;

private:
	void SendPayloadInternal();
	void OnHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	bool ParseNIMResponse(const FString& ResponseString, FString& OutContent, FString& OutReasoning, TArray<FAIToolCall>& OutToolCalls, FString& OutError);

	FHttpRequestPtr ActiveHttpRequest;
	FOnNIMResponseReceived PendingResponseDelegate;
	FOnNIMError PendingErrorDelegate;

	FString CachedPayloadString;
	int32 CurrentRetryCount;
	int32 MaxRetryAttempts;
};
