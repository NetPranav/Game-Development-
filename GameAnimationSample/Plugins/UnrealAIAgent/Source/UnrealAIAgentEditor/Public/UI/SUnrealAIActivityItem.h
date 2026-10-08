#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UnrealAIAgentTypes.h"

struct FAIToolStepData
{
	FString ToolName;
	FString Summary;
	FString ToolArgsJson;
	FString ToolResultJson;
	bool bSuccess;
	bool bIsRunning;

	FAIToolStepData()
		: ToolName(TEXT(""))
		, Summary(TEXT(""))
		, ToolArgsJson(TEXT(""))
		, ToolResultJson(TEXT(""))
		, bSuccess(true)
		, bIsRunning(false)
	{
	}
};

struct FAITurnData
{
	FString UserPrompt;
	FDateTime Timestamp;
	FString ThinkingText;
	FString PlanText;
	TArray<FAIToolStepData> ToolSteps;
	FString FinalSummary;
	EUnrealAIAgentStatus Status;
	FString LiveStatusText;
	bool bSuccess;
	bool bIsCompleted;

	FAITurnData()
		: UserPrompt(TEXT(""))
		, Timestamp(FDateTime::Now())
		, ThinkingText(TEXT(""))
		, PlanText(TEXT(""))
		, FinalSummary(TEXT(""))
		, Status(EUnrealAIAgentStatus::Idle)
		, LiveStatusText(TEXT(""))
		, bSuccess(true)
		, bIsCompleted(false)
	{
	}
};

class UNREALAIAGENTEDITOR_API SUnrealAIActivityItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealAIActivityItem) {}
		SLATE_ARGUMENT(TSharedPtr<FAITurnData>, TurnData)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void RefreshContent();

private:
	FText GetLiveStatusText() const;
	EVisibility GetLiveSpinnerVisibility() const;
	EVisibility GetThinkingVisibility() const;
	EVisibility GetCompletionVisibility() const;

	FReply OnCopyTextClicked(FString InTextToCopy);

	TSharedPtr<FAITurnData> Data;
	TSharedPtr<SVerticalBox> MainContentBox;
	TSharedPtr<SVerticalBox> ToolStepsContainer;
	TSharedPtr<STextBlock> LiveStatusTextBlock;

	double AnimationTime;
	int32 SpinnerFrameIndex;
};

