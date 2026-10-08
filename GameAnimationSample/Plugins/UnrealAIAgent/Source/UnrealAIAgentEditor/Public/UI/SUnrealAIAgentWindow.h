// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Orchestrator/UnrealAIAgentOrchestrator.h"
#include "UI/SUnrealAIActivityItem.h"

struct FAIChatSession
{
	FGuid SessionId;
	FString Title;
	FDateTime Timestamp;
	TArray<TSharedPtr<FAITurnData>> Turns;
	TArray<FAIChatMessage> LLMHistory;

	FAIChatSession()
		: SessionId(FGuid::NewGuid())
		, Title(TEXT("New Chat"))
		, Timestamp(FDateTime::Now())
	{
	}
};

class UNREALAIAGENTEDITOR_API SUnrealAIAgentWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealAIAgentWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SUnrealAIAgentWindow() override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// Orchestrator callbacks
	void OnStatusChanged(EUnrealAIAgentStatus NewStatus);
	void OnActivity(const FString& Title, const FString& Detail);
	void OnToolExecuted(const FAIToolCall& ToolCall, const FAIToolResult& Result);
	void OnCompleted(bool bSuccess, const FString& FinalSummary);

	// Action handlers
	FReply OnExecuteClicked();
	FReply OnCancelClicked();
	FReply OnNewChatClicked();
	FReply OnMoreOptionsClicked();
	FReply OnOpenSettingsClicked();
	FReply OnPlusActionsClicked();
	void OnSuggestionClicked(FString PromptText);

	// Session History Management
	void SaveCurrentSession();
	void LoadSession(TSharedPtr<FAIChatSession> SessionToLoad);
	void ClearAllSessions();

	// Key handling
	FReply OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	// UI Layout Builders
	void RebuildMainView();
	TSharedRef<SWidget> BuildTopHeader();
	TSharedRef<SWidget> BuildEmptyStateView();
	TSharedRef<SWidget> BuildActiveChatView();
	TSharedRef<SWidget> BuildInputCard(bool bInEmptyState);
	TSharedRef<SWidget> BuildFooterBar();

	// Dropdowns and menus
	TSharedRef<SWidget> MakeModelPickerMenu();
	TSharedRef<SWidget> MakePlusContextMenu();
	TSharedRef<SWidget> MakeHistoryMenu();
	void SetSelectedModel(FString InModelId);

	// Helpers
	void RefreshActiveTurnWidget();
	FText GetHeaderTitleText() const;
	FText GetActiveModelDisplayName() const;
	FText GetStatusText() const;
	FSlateColor GetStatusColor() const;
	EVisibility GetLiveSpinnerVisibility() const;
	bool IsExecuteEnabled() const;
	bool IsCancelEnabled() const;

	TSharedPtr<SVerticalBox> MainContentBox;
	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	TSharedPtr<SScrollBox> ActivityScrollBox;
	TSharedPtr<STextBlock> HeaderTitleBlock;
	TSharedPtr<SComboButton> ModelComboButton;

	TArray<TSharedPtr<FAIChatSession>> SavedSessions;
	TSharedPtr<FAIChatSession> CurrentSession;
	TArray<TSharedPtr<FAITurnData>> TurnHistory;
	TSharedPtr<FAITurnData> ActiveTurn;
	TArray<TSharedPtr<SUnrealAIActivityItem>> TurnWidgets;

	double ActiveElapsedTime;
	int32 SpinnerFrameIndex;
};


