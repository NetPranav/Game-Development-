// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NIM/UnrealNIMClient.h"
#include "Registry/UnrealAIToolRegistry.h"
#include "UnrealAIAgentTypes.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentStatusChanged, EUnrealAIAgentStatus /*NewStatus*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentActivity, const FString& /*Title*/, const FString& /*Detail*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentToolExecuted, const FAIToolCall& /*ToolCall*/, const FAIToolResult& /*Result*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentCompleted, bool /*bSuccess*/, const FString& /*FinalSummary*/);

/**
 * Orchestrates multi-step reasoning, tool dispatching, Blueprint compilation inspection,
 * error self-correction repair loops, and editor transactions.
 */
class UNREALAIAGENTEDITOR_API FUnrealAIAgentOrchestrator : public TSharedFromThis<FUnrealAIAgentOrchestrator>
{
public:
	FUnrealAIAgentOrchestrator();
	~FUnrealAIAgentOrchestrator();

	static FUnrealAIAgentOrchestrator& Get();

	/** Start an AI task with natural language prompt */
	bool ExecuteUserRequest(const FString& UserPrompt);

	/** Cancel ongoing execution */
	void CancelExecution();

	/** Reset conversation history and state */
	void ClearHistory();

	/** Set active conversation history (for resuming saved sessions) */
	void SetConversationHistory(const TArray<FAIChatMessage>& InHistory);

	/** Get current status */
	EUnrealAIAgentStatus GetStatus() const { return CurrentStatus; }

	/** Get conversation history */
	const TArray<FAIChatMessage>& GetConversationHistory() const { return ConversationHistory; }

	/** Events */
	FOnAgentStatusChanged OnStatusChanged;
	FOnAgentActivity OnActivity;
	FOnAgentToolExecuted OnToolExecuted;
	FOnAgentCompleted OnCompleted;

private:
	void SetStatus(EUnrealAIAgentStatus NewStatus);
	void DispatchNextStep();
	void OnNIMResponse(const FString& AssistantReply, const FString& ReasoningContent, const TArray<FAIToolCall>& ToolCalls, bool bSuccess);
	void OnNIMError(const FString& ErrorMessage);
	void ExecuteToolCalls(const TArray<FAIToolCall>& ToolCalls);
	void FinishExecution(bool bSuccess, const FString& FinalSummary);
	FString BuildSystemPrompt() const;

	TSharedPtr<FUnrealNIMClient> NIMClient;
	TArray<FAIChatMessage> ConversationHistory;
	EUnrealAIAgentStatus CurrentStatus;
	int32 CurrentStep;
	int32 CurrentRepairAttempts;
	bool bInTransaction;
};
