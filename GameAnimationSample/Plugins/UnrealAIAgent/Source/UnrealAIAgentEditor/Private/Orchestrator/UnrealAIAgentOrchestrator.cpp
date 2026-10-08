// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Orchestrator/UnrealAIAgentOrchestrator.h"
#include "UnrealAIAgentSettings.h"
#include "UnrealAIAgentEditorModule.h"
#include "Editor.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "FUnrealAIAgentOrchestrator"

FUnrealAIAgentOrchestrator::FUnrealAIAgentOrchestrator()
	: CurrentStatus(EUnrealAIAgentStatus::Idle)
	, CurrentStep(0)
	, CurrentRepairAttempts(0)
	, bInTransaction(false)
{
	NIMClient = MakeShared<FUnrealNIMClient>();
}

FUnrealAIAgentOrchestrator::~FUnrealAIAgentOrchestrator()
{
	CancelExecution();
}

FUnrealAIAgentOrchestrator& FUnrealAIAgentOrchestrator::Get()
{
	static TSharedRef<FUnrealAIAgentOrchestrator> Instance = MakeShared<FUnrealAIAgentOrchestrator>();
	return *Instance;
}

void FUnrealAIAgentOrchestrator::SetStatus(EUnrealAIAgentStatus NewStatus)
{
	CurrentStatus = NewStatus;
	OnStatusChanged.Broadcast(NewStatus);
}

FString FUnrealAIAgentOrchestrator::BuildSystemPrompt() const
{
	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	if (Settings && !Settings->SystemPromptOverride.IsEmpty())
	{
		return Settings->SystemPromptOverride;
	}

	FString ProjectName = FApp::GetProjectName();
	if (ProjectName.IsEmpty())
	{
		ProjectName = TEXT("Unreal Project");
	}

	return FString::Printf(
		TEXT("You are the Unreal Engine AI Agent for project '%s', an autonomous assistant integrated directly into Unreal Editor.\n")
		TEXT("Your mission is to understand user requests in natural language, formulate structured plans, and execute them using the provided Unreal Editor tools.\n\n")
		TEXT("CRITICAL RULES & OPERATIONAL PRINCIPLES:\n")
		TEXT("1. MULTI-TURN CONVERSATION & MEMORY: You maintain full conversational memory across multiple turns in this project. When the user asks follow-up questions referencing earlier created assets, Blueprints, variables, or functions (e.g. 'now add a particle effect to it', 'increase damage to 50', 'what variables did we create?'), you MUST reference and modify the existing assets from earlier in the conversation rather than recreating them from scratch.\n")
		TEXT("2. TOOL EXECUTION: You have access to safe, structured Unreal Editor tools for assets, Blueprints, graphs, pins, nodes, variables, and components. NEVER attempt arbitrary code execution.\n")
		TEXT("3. When generating a new Blueprint:\n")
		TEXT("   - Create asset folder if needed (e.g. create_asset_folder '/Game/Weapons').\n")
		TEXT("   - Create the Blueprint (e.g. create_blueprint 'BP_Sword', '/Game/Weapons', 'Actor').\n")
		TEXT("   - Add required components (e.g. add_component 'StaticMeshComponent', 'SwordMesh').\n")
		TEXT("   - Set component properties (e.g. set_component_property assigning static mesh).\n")
		TEXT("   - Create variables with types and default values (e.g. create_variable 'Damage', 'Float', '35.0').\n")
		TEXT("   - Create graph nodes (e.g. create_node 'CustomEvent' 'Attack', create_node 'PrintString', create_node 'VariableGet' 'Damage').\n")
		TEXT("   - Inspect pins (get_node_pins) to ensure exact pin names before connecting.\n")
		TEXT("   - Connect pins (connect_pins) between execution and data pins.\n")
		TEXT("   - ALWAYS compile the Blueprint (compile_blueprint) and check results.\n")
		TEXT("   - If compilation fails, analyze compiler error messages and use repair tools to fix connections or types.\n")
		TEXT("   - Save the asset (save_blueprint or save_asset).\n")
		TEXT("4. When modifying an existing Blueprint:\n")
		TEXT("   - Inspect Blueprint structure (inspect_blueprint) and graph nodes (get_graph_nodes).\n")
		TEXT("   - Find relevant nodes (find_node) and examine pins (get_node_pins).\n")
		TEXT("   - Make precise modifications, delete/create nodes, reconnect pins.\n")
		TEXT("   - Compile and verify before reporting completion.\n")
		TEXT("5. Be concise and structured in your text explanations. Keep intermediate planning concise."),
		*ProjectName
	);
}


bool FUnrealAIAgentOrchestrator::ExecuteUserRequest(const FString& UserPrompt)
{
	if (UserPrompt.IsEmpty() || CurrentStatus == EUnrealAIAgentStatus::Planning || CurrentStatus == EUnrealAIAgentStatus::ExecutingTool)
	{
		return false;
	}

	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	if (Settings && Settings->GetEffectiveApiKey().IsEmpty())
	{
		OnActivity.Broadcast(TEXT("Configuration Error"), TEXT("NVIDIA NIM API Key is missing. Please set it in Project Settings -> Unreal AI Agent."));
		SetStatus(EUnrealAIAgentStatus::Failed);
		return false;
	}

	CurrentStep = 0;
	CurrentRepairAttempts = 0;

	if (ConversationHistory.Num() == 0)
	{
		ConversationHistory.Add(FAIChatMessage::MakeSystem(BuildSystemPrompt()));
	}

	ConversationHistory.Add(FAIChatMessage::MakeUser(UserPrompt));
	OnActivity.Broadcast(TEXT("User Prompt"), UserPrompt);

	if (Settings && Settings->bEnableTransactions && GEditor && !bInTransaction)
	{
		GEditor->BeginTransaction(LOCTEXT("AIAgentAction", "Unreal AI Agent Action"));
		bInTransaction = true;
	}

	DispatchNextStep();
	return true;
}

void FUnrealAIAgentOrchestrator::DispatchNextStep()
{
	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	const int32 MaxSteps = Settings ? Settings->MaxSteps : 25;

	if (CurrentStep >= MaxSteps)
	{
		FinishExecution(true, FString::Printf(TEXT("Reached maximum step limit (%d). Completed actions."), MaxSteps));
		return;
	}

	SetStatus(EUnrealAIAgentStatus::Planning);

	TArray<TSharedPtr<FJsonValue>> ToolsJson = FUnrealAIToolRegistry::Get().GenerateNIMToolsJson();

	NIMClient->SendChatCompletion(
		ConversationHistory,
		ToolsJson,
		FOnNIMResponseReceived::CreateSP(this, &FUnrealAIAgentOrchestrator::OnNIMResponse),
		FOnNIMError::CreateSP(this, &FUnrealAIAgentOrchestrator::OnNIMError)
	);
}

void FUnrealAIAgentOrchestrator::OnNIMResponse(const FString& AssistantReply, const FString& ReasoningContent, const TArray<FAIToolCall>& ToolCalls, bool bSuccess)
{
	if (!ReasoningContent.IsEmpty())
	{
		OnActivity.Broadcast(TEXT("AI Thinking"), ReasoningContent);
	}

	if (!AssistantReply.IsEmpty())
	{
		OnActivity.Broadcast(TEXT("AI Plan"), AssistantReply);
	}

	ConversationHistory.Add(FAIChatMessage::MakeAssistant(AssistantReply, ToolCalls));

	if (ToolCalls.Num() > 0)
	{
		ExecuteToolCalls(ToolCalls);
	}
	else
	{
		// LLM finished without further tool calls
		FinishExecution(true, AssistantReply.IsEmpty() ? TEXT("Task completed successfully.") : AssistantReply);
	}
}

void FUnrealAIAgentOrchestrator::ExecuteToolCalls(const TArray<FAIToolCall>& ToolCalls)
{
	SetStatus(EUnrealAIAgentStatus::ExecutingTool);

	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	const int32 MaxRepair = Settings ? Settings->MaxRepairAttempts : 5;

	bool bHadCompileError = false;
	FString CompileErrorMessage;

	for (const FAIToolCall& ToolCall : ToolCalls)
	{
		FAIToolResult Result = FUnrealAIToolRegistry::Get().ExecuteToolCall(ToolCall);

		OnToolExecuted.Broadcast(ToolCall, Result);
		OnActivity.Broadcast(
			FString::Printf(TEXT("Tool: %s"), *ToolCall.Name),
			Result.bSuccess ? TEXT("Success") : FString::Printf(TEXT("Error: %s"), *Result.ErrorMessage)
		);

		// Record tool response in conversation
		ConversationHistory.Add(FAIChatMessage::MakeTool(ToolCall.Id, ToolCall.Name, Result.ReturnJson));

		if (ToolCall.Name.Equals(TEXT("compile_blueprint"), ESearchCase::IgnoreCase) && !Result.bSuccess)
		{
			bHadCompileError = true;
			CompileErrorMessage = Result.ErrorMessage;
		}
	}

	if (bHadCompileError)
	{
		CurrentRepairAttempts++;
		if (CurrentRepairAttempts <= MaxRepair)
		{
			SetStatus(EUnrealAIAgentStatus::Repairing);
			OnActivity.Broadcast(
				TEXT("Compiler Self-Correction"),
				FString::Printf(TEXT("Repair Attempt %d/%d: Analyzing compiler error and generating fix..."), CurrentRepairAttempts, MaxRepair)
			);
		}
	}

	CurrentStep++;
	DispatchNextStep();
}

void FUnrealAIAgentOrchestrator::OnNIMError(const FString& ErrorMessage)
{
	OnActivity.Broadcast(TEXT("NIM Request Error"), ErrorMessage);
	FinishExecution(false, ErrorMessage);
}

void FUnrealAIAgentOrchestrator::FinishExecution(bool bSuccess, const FString& FinalSummary)
{
	if (bInTransaction && GEditor)
	{
		GEditor->EndTransaction();
		bInTransaction = false;
	}

	SetStatus(bSuccess ? EUnrealAIAgentStatus::Completed : EUnrealAIAgentStatus::Failed);
	OnCompleted.Broadcast(bSuccess, FinalSummary);
}

void FUnrealAIAgentOrchestrator::CancelExecution()
{
	if (NIMClient.IsValid())
	{
		NIMClient->CancelActiveRequest();
	}

	if (bInTransaction && GEditor)
	{
		GEditor->EndTransaction();
		bInTransaction = false;
	}

	SetStatus(EUnrealAIAgentStatus::Cancelled);
	OnActivity.Broadcast(TEXT("Cancelled"), TEXT("Agent execution was cancelled by the user."));
}

void FUnrealAIAgentOrchestrator::ClearHistory()
{
	CancelExecution();
	ConversationHistory.Empty();
	CurrentStep = 0;
	CurrentRepairAttempts = 0;
	SetStatus(EUnrealAIAgentStatus::Idle);
	OnActivity.Broadcast(TEXT("Cleared"), TEXT("Conversation history and agent state cleared."));
}

void FUnrealAIAgentOrchestrator::SetConversationHistory(const TArray<FAIChatMessage>& InHistory)
{
	CancelExecution();
	ConversationHistory = InHistory;
	CurrentStep = 0;
	CurrentRepairAttempts = 0;
	SetStatus(EUnrealAIAgentStatus::Idle);
}

#undef LOCTEXT_NAMESPACE

