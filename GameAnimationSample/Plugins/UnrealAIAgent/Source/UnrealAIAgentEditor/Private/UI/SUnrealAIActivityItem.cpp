#include "UI/SUnrealAIActivityItem.h"
#include "UnrealAIAgentStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Styling/AppStyle.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "SUnrealAIActivityItem"

FReply SUnrealAIActivityItem::OnCopyTextClicked(FString InTextToCopy)
{
	FPlatformApplicationMisc::ClipboardCopy(*InTextToCopy);
	return FReply::Handled();
}


static const TCHAR* GSpinnerFrames[] = {
	TEXT("⠋"), TEXT("⠙"), TEXT("⠹"), TEXT("⠸"), TEXT("⠼"),
	TEXT("⠴"), TEXT("⠦"), TEXT("⠧"), TEXT("⠇"), TEXT("⠏")
};
static const int32 GNumSpinnerFrames = 10;

// -----------------------------------------------------------------------------
// Markdown to Slate Formatter
// -----------------------------------------------------------------------------
static TSharedRef<SWidget> RenderMarkdownToSlate(const FString& InMarkdown)
{
	TSharedRef<SVerticalBox> OutBox = SNew(SVerticalBox);
	if (InMarkdown.IsEmpty())
	{
		return OutBox;
	}

	const ISlateStyle& AIStyle = FUnrealAIAgentStyle::Get();

	TArray<FString> Lines;
	InMarkdown.ParseIntoArrayLines(Lines, false);

	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		FString Line = Lines[i].TrimStartAndEnd();
		if (Line.IsEmpty())
		{
			OutBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SSpacer)
				];
			continue;
		}

		// 1. Markdown Table Detection (| Col1 | Col2 |)
		if (Line.StartsWith(TEXT("|")) && Line.EndsWith(TEXT("|")))
		{
			TArray<FString> TableLines;
			while (i < Lines.Num() && Lines[i].TrimStartAndEnd().StartsWith(TEXT("|")) && Lines[i].TrimStartAndEnd().EndsWith(TEXT("|")))
			{
				TableLines.Add(Lines[i].TrimStartAndEnd());
				++i;
			}
			--i; // step back one

			if (TableLines.Num() >= 2)
			{
				TSharedPtr<SVerticalBox> TableBox = SNew(SVerticalBox);

				// Parse headers
				TArray<FString> HeaderCols;
				TableLines[0].ParseIntoArray(HeaderCols, TEXT("|"), false);
				for (FString& Col : HeaderCols) { Col.TrimStartAndEndInline(); }

				// Header Row Widget
				TSharedPtr<SHorizontalBox> HeaderRow = SNew(SHorizontalBox);
				for (const FString& HeaderCol : HeaderCols)
				{
					if (HeaderCol.IsEmpty()) continue;
					HeaderRow->AddSlot()
						.FillWidth(1.0f)
						.Padding(8.0f, 6.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(HeaderCol))
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFFFF")))))
						];
				}

				TableBox->AddSlot()
					.AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
						.Padding(FMargin(0.0f))
						[
							HeaderRow.ToSharedRef()
						]
					];

				// Data Rows (skip index 1 which is separator |---|---|)
				int32 StartRow = (TableLines.Num() > 1 && TableLines[1].Contains(TEXT("-"))) ? 2 : 1;
				for (int32 RowIdx = StartRow; RowIdx < TableLines.Num(); ++RowIdx)
				{
					TArray<FString> DataCols;
					TableLines[RowIdx].ParseIntoArray(DataCols, TEXT("|"), false);
					for (FString& Col : DataCols) { Col.TrimStartAndEndInline(); }

					TSharedPtr<SHorizontalBox> DataRow = SNew(SHorizontalBox);
					for (const FString& Cell : DataCols)
					{
						if (Cell.IsEmpty()) continue;
						DataRow->AddSlot()
							.FillWidth(1.0f)
							.Padding(8.0f, 4.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Cell))
								.Font(FAppStyle::GetFontStyle("Small"))
								.AutoWrapText(true)
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("C5C8D4")))))
							];
					}

					FLinearColor RowBg = (RowIdx % 2 == 0) ? FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1A1B1F"))) : FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("16171A")));

					TableBox->AddSlot()
						.AutoHeight()
						.Padding(0.0f, 1.0f)
						[
							SNew(SBorder)
							.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
							.BorderBackgroundColor(RowBg)
							.Padding(FMargin(0.0f))
							[
								DataRow.ToSharedRef()
							]
						];
				}

				OutBox->AddSlot()
					.AutoHeight()
					.Padding(0.0f, 6.0f, 0.0f, 8.0f)
					[
						SNew(SBorder)
						.BorderImage(AIStyle.GetBrush("UnrealAIAgent.CardBg"))
						.Padding(FMargin(4.0f))
						[
							TableBox.ToSharedRef()
						]
					];
			}
			continue;
		}

		// 2. Headings & Section Titles (e.g. ## Header or **Title:**)
		if (Line.StartsWith(TEXT("### ")) || Line.StartsWith(TEXT("## ")) || Line.StartsWith(TEXT("# ")))
		{
			FString CleanHeader = Line;
			CleanHeader.RemoveFromStart(TEXT("### "));
			CleanHeader.RemoveFromStart(TEXT("## "));
			CleanHeader.RemoveFromStart(TEXT("# "));
			CleanHeader.ReplaceInline(TEXT("**"), TEXT(""));

			OutBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 8.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(CleanHeader))
					.Font(FAppStyle::GetFontStyle("NormalBold"))
					.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFFFF")))))
				];
			continue;
		}

		// 3. Bullet list / Key-Value Pairs (e.g. - **Key:** Value or - Value)
		if (Line.StartsWith(TEXT("- ")) || Line.StartsWith(TEXT("* ")) || Line.StartsWith(TEXT("• ")))
		{
			FString CleanBullet = Line;
			CleanBullet.RemoveFromStart(TEXT("- "));
			CleanBullet.RemoveFromStart(TEXT("* "));
			CleanBullet.RemoveFromStart(TEXT("• "));

			int32 FirstBold = CleanBullet.Find(TEXT("**"));
			int32 SecondBold = (FirstBold != INDEX_NONE) ? CleanBullet.Find(TEXT("**"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FirstBold + 2) : INDEX_NONE;

			if (FirstBold == 0 && SecondBold != INDEX_NONE)
			{
				FString KeyName = CleanBullet.Mid(2, SecondBold - 2);
				FString ValueStr = CleanBullet.Mid(SecondBold + 2).TrimStartAndEnd();
				ValueStr.ReplaceInline(TEXT("**"), TEXT(""));
				ValueStr.ReplaceInline(TEXT("`"), TEXT(""));

				OutBox->AddSlot()
					.AutoHeight()
					.Padding(4.0f, 2.0f, 0.0f, 2.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						.Padding(0.0f, 1.0f, 6.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("•")))
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0084FF")))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						.Padding(0.0f, 0.0f, 4.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(KeyName))
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E6E9F2")))))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.Text(FText::FromString(ValueStr))
							.Font(FAppStyle::GetFontStyle("Small"))
							.AutoWrapText(true)
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("A8ACB8")))))
						]
					];
				continue;
			}
			else
			{
				CleanBullet.ReplaceInline(TEXT("**"), TEXT(""));
				CleanBullet.ReplaceInline(TEXT("`"), TEXT(""));

				OutBox->AddSlot()
					.AutoHeight()
					.Padding(4.0f, 2.0f, 0.0f, 2.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						.Padding(0.0f, 1.0f, 6.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("•")))
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0084FF")))))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.Text(FText::FromString(CleanBullet))
							.Font(FAppStyle::GetFontStyle("Small"))
							.AutoWrapText(true)
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("C8CBD6")))))
						]
					];
				continue;
			}
		}

		// 4. Standalone Bold Titles (e.g. **Blueprint Count:** ...)
		if (Line.StartsWith(TEXT("**")) && Line.EndsWith(TEXT("**")))
		{
			FString Title = Line;
			Title.ReplaceInline(TEXT("**"), TEXT(""));

			OutBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 3.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Title))
					.Font(FAppStyle::GetFontStyle("NormalBold"))
					.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFFFF")))))
				];
			continue;
		}

		// 5. Standard paragraph
		FString CleanParagraph = Line;
		CleanParagraph.ReplaceInline(TEXT("**"), TEXT(""));
		CleanParagraph.ReplaceInline(TEXT("`"), TEXT(""));

		OutBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 3.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CleanParagraph))
				.Font(FAppStyle::GetFontStyle("Normal"))
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("DDE0EC")))))
			];
	}

	return OutBox;
}

void SUnrealAIActivityItem::Construct(const FArguments& InArgs)
{
	Data = InArgs._TurnData;
	AnimationTime = 0.0;
	SpinnerFrameIndex = 0;

	if (!Data.IsValid())
	{
		return;
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 6.0f)
		[
			SAssignNew(MainContentBox, SVerticalBox)
		]
	];

	RefreshContent();
}

void SUnrealAIActivityItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (Data.IsValid() && !Data->bIsCompleted && Data->Status != EUnrealAIAgentStatus::Idle)
	{
		AnimationTime += InDeltaTime;
		SpinnerFrameIndex = (int32)(AnimationTime * 12.0) % GNumSpinnerFrames;

		if (LiveStatusTextBlock.IsValid())
		{
			LiveStatusTextBlock->SetText(GetLiveStatusText());
		}
	}
}

FText SUnrealAIActivityItem::GetLiveStatusText() const
{
	if (!Data.IsValid())
	{
		return FText::GetEmpty();
	}

	FString Frame = GSpinnerFrames[SpinnerFrameIndex % GNumSpinnerFrames];
	FString StatusStr = Data->LiveStatusText;
	if (StatusStr.IsEmpty())
	{
		switch (Data->Status)
		{
		case EUnrealAIAgentStatus::Planning:
			StatusStr = TEXT("Reasoning & Planning...");
			break;
		case EUnrealAIAgentStatus::ExecutingTool:
			StatusStr = TEXT("Executing Unreal Editor operations...");
			break;
		case EUnrealAIAgentStatus::Compiling:
			StatusStr = TEXT("Compiling Blueprints & verifying schema...");
			break;
		case EUnrealAIAgentStatus::Repairing:
			StatusStr = TEXT("Self-correcting compiler diagnostics...");
			break;
		default:
			StatusStr = TEXT("Processing...");
			break;
		}
	}

	return FText::FromString(FString::Printf(TEXT("%s  %s"), *Frame, *StatusStr));
}

EVisibility SUnrealAIActivityItem::GetLiveSpinnerVisibility() const
{
	if (Data.IsValid() && !Data->bIsCompleted && Data->Status != EUnrealAIAgentStatus::Idle)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SUnrealAIActivityItem::GetThinkingVisibility() const
{
	if (Data.IsValid() && !Data->ThinkingText.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SUnrealAIActivityItem::GetCompletionVisibility() const
{
	if (Data.IsValid() && Data->bIsCompleted)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void SUnrealAIActivityItem::RefreshContent()
{
	if (!MainContentBox.IsValid() || !Data.IsValid())
	{
		return;
	}

	MainContentBox->ClearChildren();

	const ISlateStyle& AIStyle = FUnrealAIAgentStyle::Get();

	// 1. User Prompt Bubble (Image 3 style: dark rounded card with subtle border and Copy button)
	MainContentBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderImage(AIStyle.GetBrush("UnrealAIAgent.UserBubbleBg"))
			.Padding(FMargin(14.0f, 10.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Data->UserPrompt))
					.Font(FAppStyle::GetFontStyle("Normal"))
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F0F2F8")))))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Top)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
					.ContentPadding(FMargin(4.0f, 2.0f))
					.ToolTipText(LOCTEXT("CopyPromptTip", "Copy prompt to clipboard"))
					.OnClicked(this, &SUnrealAIActivityItem::OnCopyTextClicked, Data->UserPrompt)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("📋")))
						.Font(FAppStyle::GetFontStyle("Small"))
						.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7E8392")))))
					]
				]
			]
		];

	// 2. Prominent Real-Time Planning & Execution Card (Visible while running)
	if (!Data->bIsCompleted && Data->Status != EUnrealAIAgentStatus::Idle)
	{
		MainContentBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderImage(AIStyle.GetBrush("UnrealAIAgent.CardBg"))
				.Padding(FMargin(12.0f, 10.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("⚡")))
						.Font(FAppStyle::GetFontStyle("Bold"))
						.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0084FF")))))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(LiveStatusTextBlock, STextBlock)
						.Text(this, &SUnrealAIActivityItem::GetLiveStatusText)
						.Font(FAppStyle::GetFontStyle("NormalBold"))
						.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E8EBF5")))))
					]
				]
			];
	}

	// 3. Tool / Attachment Chips Row
	if (Data->ToolSteps.Num() > 0 || !Data->ThinkingText.IsEmpty() || !Data->PlanText.IsEmpty())
	{
		TSharedPtr<SHorizontalBox> BadgeRow = SNew(SHorizontalBox);

		BadgeRow->AddSlot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
				.Padding(FMargin(8.0f, 3.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("📄")))
						.Font(FAppStyle::GetFontStyle("Small"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Editor Tools")))
						.Font(FAppStyle::GetFontStyle("Small"))
						.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("9EA3B2")))))
					]
				]
			];

		if (!Data->bIsCompleted)
		{
			BadgeRow->AddSlot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
					.Padding(FMargin(8.0f, 3.0f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, 4.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("⚡")))
							.Font(FAppStyle::GetFontStyle("Small"))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Running Plan...")))
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0084FF")))))
						]
					]
				];
		}

		MainContentBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				BadgeRow.ToSharedRef()
			];
	}

	// 4. AI Thinking & Planning Box (Collapsible Dropdown)
	FString CombinedThought = Data->ThinkingText;
	if (CombinedThought.IsEmpty() && !Data->PlanText.IsEmpty() && !Data->bIsCompleted)
	{
		CombinedThought = Data->PlanText;
	}

	if (!CombinedThought.IsEmpty())
	{
		MainContentBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
				.Padding(FMargin(10.0f, 6.0f))
				[
					SNew(SExpandableArea)
					.InitiallyCollapsed(!Data->bIsCompleted)
					.HeaderContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("🧠")))
							.Font(FAppStyle::GetFontStyle("Small"))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Thought Process & Planning (Click to expand)")))
							.Font(FAppStyle::GetFontStyle("Small"))
							.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8E9B")))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SButton)
							.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
							.ContentPadding(FMargin(4.0f, 2.0f))
							.ToolTipText(LOCTEXT("CopyThinkingTip", "Copy thinking process to clipboard"))
							.OnClicked(this, &SUnrealAIActivityItem::OnCopyTextClicked, CombinedThought)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("📋")))
								.Font(FAppStyle::GetFontStyle("Small"))
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7E8392")))))
							]
						]
					]
					.BodyContent()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							RenderMarkdownToSlate(CombinedThought)
						]
					]
				]
			];
	}

	// 5. Executed Tools Timeline
	if (Data->ToolSteps.Num() > 0)
	{
		TSharedPtr<SVerticalBox> ToolsBox = SNew(SVerticalBox);

		for (const FAIToolStepData& Step : Data->ToolSteps)
		{
			FLinearColor StepColor = Step.bSuccess ? FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2ED573"))) : FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF4757")));
			FString StatusIcon = Step.bSuccess ? TEXT("✓") : TEXT("✗");
			FString StepCombinedDetails = FString::Printf(TEXT("Args:\n%s\n\nResult:\n%s"), *Step.ToolArgsJson, *Step.ToolResultJson);

			ToolsBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderImage(AIStyle.GetBrush("UnrealAIAgent.ChipBg"))
					.Padding(FMargin(8.0f, 5.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(StatusIcon))
								.Font(FAppStyle::GetFontStyle("Bold"))
								.ColorAndOpacity(FSlateColor(StepColor))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Step.ToolName))
								.Font(FAppStyle::GetFontStyle("Bold"))
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFFFF")))))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Step.Summary))
								.Font(FAppStyle::GetFontStyle("Normal"))
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("A2A6B4")))))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SNew(SExpandableArea)
							.InitiallyCollapsed(true)
							.HeaderContent()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Details")))
									.Font(FAppStyle::GetFontStyle("Small"))
									.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("686C78")))))
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SButton)
									.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
									.ContentPadding(FMargin(4.0f, 2.0f))
									.ToolTipText(LOCTEXT("CopyDetailsTip", "Copy tool details to clipboard"))
									.OnClicked(this, &SUnrealAIActivityItem::OnCopyTextClicked, StepCombinedDetails)
									[
										SNew(STextBlock)
										.Text(FText::FromString(TEXT("📋")))
										.Font(FAppStyle::GetFontStyle("Small"))
										.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("686C78")))))
									]
								]
							]
							.BodyContent()
							[
								SNew(STextBlock)
								.Text(FText::FromString(StepCombinedDetails))
								.AutoWrapText(true)
								.Font(FAppStyle::GetFontStyle("Small"))
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7ED6DF")))))
							]
						]
					]
				];
		}

		MainContentBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				ToolsBox.ToSharedRef()
			];
	}

	// 6. Formatted Assistant Output & Action Bar
	FString MainOutputText = Data->bIsCompleted ? Data->FinalSummary : (Data->ToolSteps.Num() == 0 ? Data->PlanText : TEXT(""));
	if (!MainOutputText.IsEmpty())
	{
		MainContentBox->AddSlot()
			.AutoHeight()
			.Padding(2.0f, 4.0f, 2.0f, 4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					RenderMarkdownToSlate(MainOutputText)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
						.ContentPadding(FMargin(6.0f, 3.0f))
						.ToolTipText(LOCTEXT("CopyResponseTip", "Copy full response to clipboard"))
						.OnClicked(this, &SUnrealAIActivityItem::OnCopyTextClicked, MainOutputText)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("📋")))
								.Font(FAppStyle::GetFontStyle("Small"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CopyResponseLabel", "Copy Response"))
								.Font(FAppStyle::GetFontStyle("Small"))
								.ColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8E9C")))))
							]
						]
					]
				]
			];
	}
}

#undef LOCTEXT_NAMESPACE

