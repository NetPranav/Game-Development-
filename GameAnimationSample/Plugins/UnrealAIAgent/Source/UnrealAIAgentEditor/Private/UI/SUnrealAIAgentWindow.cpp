// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UI/SUnrealAIAgentWindow.h"
#include "UnrealAIAgentStyle.h"
#include "UnrealAIAgentSettings.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SUnrealAIAgentWindow"

void SUnrealAIAgentWindow::Construct(const FArguments& InArgs)
{
	ActiveElapsedTime = 0.0;
	SpinnerFrameIndex = 0;

	FUnrealAIAgentOrchestrator& Orchestrator = FUnrealAIAgentOrchestrator::Get();
	Orchestrator.OnStatusChanged.AddSP(this, &SUnrealAIAgentWindow::OnStatusChanged);
	Orchestrator.OnActivity.AddSP(this, &SUnrealAIAgentWindow::OnActivity);
	Orchestrator.OnToolExecuted.AddSP(this, &SUnrealAIAgentWindow::OnToolExecuted);
	Orchestrator.OnCompleted.AddSP(this, &SUnrealAIAgentWindow::OnCompleted);

	const ISlateStyle& AIStyle = FUnrealAIAgentStyle::Get();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(AIStyle.GetBrush("UnrealAIAgent.WindowBg"))
		.Padding(FMargin(12.0f, 8.0f))
		[
			SNew(SVerticalBox)

			// 1. Top Header Bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				BuildTopHeader()
			]

			// 2. Dynamic Main View Area (Empty State vs Active Chat State)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(MainContentBox, SVerticalBox)
			]

			// 3. Bottom Footer Bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				BuildFooterBar()
			]
		]
	];

	RebuildMainView();
}

SUnrealAIAgentWindow::~SUnrealAIAgentWindow()
{
	FUnrealAIAgentOrchestrator& Orchestrator = FUnrealAIAgentOrchestrator::Get();
	Orchestrator.OnStatusChanged.RemoveAll(this);
	Orchestrator.OnActivity.RemoveAll(this);
	Orchestrator.OnToolExecuted.RemoveAll(this);
	Orchestrator.OnCompleted.RemoveAll(this);
}

void SUnrealAIAgentWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	EUnrealAIAgentStatus CurrentStatus = FUnrealAIAgentOrchestrator::Get().GetStatus();
	if (CurrentStatus != EUnrealAIAgentStatus::Idle && CurrentStatus != EUnrealAIAgentStatus::Completed && CurrentStatus != EUnrealAIAgentStatus::Failed)
	{
		ActiveElapsedTime += InDeltaTime;
		SpinnerFrameIndex = (int32)(ActiveElapsedTime * 10.0) % 10;
	}
}

void SUnrealAIAgentWindow::RebuildMainView()
{
	if (!MainContentBox.IsValid())
	{
		return;
	}

	MainContentBox->ClearChildren();

	if (TurnHistory.Num() == 0)
	{
		// Empty / Idle State (Image 2)
		MainContentBox->AddSlot()
			.FillHeight(1.0f)
			[
				BuildEmptyStateView()
			];
	}
	else
	{
		// Active Chat / History State (Image 3)
		MainContentBox->AddSlot()
			.FillHeight(1.0f)
			[
				BuildActiveChatView()
			];
	}
}

TSharedRef<SWidget> SUnrealAIAgentWindow::BuildTopHeader()
{
	const ISlateStyle& AIStyle = FUnrealAIAgentStyle::Get();

	return SNew(SHorizontalBox)
		// Title (e.g., "Agent" or active prompt summary)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(HeaderTitleBlock, STextBlock)
			.Text(this, &SUnrealAIAgentWindow::GetHeaderTitleText)
			.Font(FAppStyle::GetFontStyle("NormalBold"))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.93f, 0.96f)))
		]

		// Action Icons (+ New Chat, 🕒 History, ... Settings, ✕ Reset)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(6.0f, 4.0f))
			.ToolTipText(LOCTEXT("NewChatTooltip", "New Chat / Clear"))
			.OnClicked(this, &SUnrealAIAgentWindow::OnNewChatClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("+")))
				.Font(FAppStyle::GetFontStyle("Bold"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.7f, 0.75f)))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SComboButton)
			.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(6.0f, 4.0f))
			.ToolTipText(LOCTEXT("HistoryTooltip", "Conversation History & Sessions"))
			.HasDownArrow(false)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("🕒")))
				.Font(FAppStyle::GetFontStyle("Small"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.7f, 0.75f)))
			]
			.OnGetMenuContent(this, &SUnrealAIAgentWindow::MakeHistoryMenu)
		]


		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(6.0f, 4.0f))
			.ToolTipText(LOCTEXT("SettingsTooltip", "Plugin Settings & Models"))
			.OnClicked(this, &SUnrealAIAgentWindow::OnMoreOptionsClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("•••")))
				.Font(FAppStyle::GetFontStyle("Small"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.7f, 0.75f)))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(6.0f, 4.0f))
			.ToolTipText(LOCTEXT("CloseTooltip", "Reset Agent Panel"))
			.OnClicked(this, &SUnrealAIAgentWindow::OnNewChatClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("✕")))
				.Font(FAppStyle::GetFontStyle("Small"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.7f, 0.75f)))
			]
		];
}

TSharedRef<SWidget> SUnrealAIAgentWindow::BuildEmptyStateView()
{
	FString ProjectName = FApp::GetProjectName();
	if (ProjectName.IsEmpty())
	{
		ProjectName = TEXT("Game");
	}

	return SNew(SVerticalBox)
		// Spacer to push content towards middle
		+ SVerticalBox::Slot()
		.FillHeight(0.35f)
		[
			SNew(SSpacer)
		]

		// Section Title ("Game" or Project Name)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f, 4.0f, 12.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(ProjectName))
			.Font(FAppStyle::GetFontStyle("HeadingLarge"))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.96f, 0.98f)))
		]

		// Floating Centered Modern Prompt Card (Image 2 style)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildInputCard(true)
		]

		// Disclaimer text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 12.0f, 4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Disclaimer", "AI may make mistakes. Double-check all generated code."))
			.Font(FAppStyle::GetFontStyle("Small"))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.42f, 0.44f, 0.48f)))
		]

		// Bottom spacer
		+ SVerticalBox::Slot()
		.FillHeight(0.65f)
		[
			SNew(SSpacer)
		];
}

TSharedRef<SWidget> SUnrealAIAgentWindow::BuildActiveChatView()
{
	TurnWidgets.Empty();

	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	ActivityScrollBox = ScrollBox;

	for (const TSharedPtr<FAITurnData>& Item : TurnHistory)
	{
		TSharedRef<SUnrealAIActivityItem> ItemWidget = SNew(SUnrealAIActivityItem).TurnData(Item);
		TurnWidgets.Add(ItemWidget);

		ScrollBox->AddSlot()
			.Padding(0.0f, 4.0f)
			[
				ItemWidget
			];
	}

	ScrollBox->ScrollToEnd();

	return SNew(SVerticalBox)
		// Scrollable Message List
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			ScrollBox
		]

		// Docked Bottom Prompt Card
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildInputCard(false)
		];
}

TSharedRef<SWidget> SUnrealAIAgentWindow::BuildInputCard(bool bInEmptyState)
{
	const ISlateStyle& AIStyle = FUnrealAIAgentStyle::Get();

	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	bool bIsRunning = (Status != EUnrealAIAgentStatus::Idle && Status != EUnrealAIAgentStatus::Completed && Status != EUnrealAIAgentStatus::Failed);

	return SNew(SBorder)
		.BorderImage(AIStyle.GetBrush("UnrealAIAgent.CardBg"))
		.Padding(FMargin(12.0f, 10.0f, 12.0f, 8.0f))
		[
			SNew(SVerticalBox)

			// 1. Text Input Area
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SAssignNew(InputTextBox, SMultiLineEditableTextBox)
				.Style(&AIStyle.GetWidgetStyle<FEditableTextBoxStyle>("UnrealAIAgent.InputBox"))
				.HintText(LOCTEXT("InputHintText", "Ask anything, @ to mention, / for actions"))
				.AutoWrapText(true)
				.ForegroundColor(FSlateColor(FLinearColor(0.92f, 0.93f, 0.96f)))
				.OnKeyDownHandler(this, &SUnrealAIAgentWindow::OnInputKeyDown)

			]

			// 2. Card Bottom Action Bar (+ Context, Model Pill Selector, Mic Icon, Blue Circular Send Button)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// + Context / Tools Button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SComboButton)
					.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
					.ContentPadding(FMargin(6.0f, 4.0f))
					.ToolTipText(LOCTEXT("PlusActionsTooltip", "Insert Blueprint & Project Prompt Templates"))
					.HasDownArrow(false)
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("+")))
						.Font(FAppStyle::GetFontStyle("Bold"))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.7f, 0.75f)))
					]
					.OnGetMenuContent(this, &SUnrealAIAgentWindow::MakePlusContextMenu)
				]

				// Model Pill Dropdown (e.g. Gemini 3.7 Flash High ⌄)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SAssignNew(ModelComboButton, SComboButton)
					.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.ModelPill"))
					.ContentPadding(FMargin(8.0f, 4.0f))
					.ToolTipText(LOCTEXT("ModelPickerTooltip", "Choose AI Model"))
					.HasDownArrow(false)
					.ButtonContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, 4.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(this, &SUnrealAIAgentWindow::GetActiveModelDisplayName)
							.Font(FAppStyle::GetFontStyle("SmallBold"))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.8f, 0.85f)))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("⌄")))
							.Font(FAppStyle::GetFontStyle("Small"))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.58f, 0.65f)))
						]
					]
					.OnGetMenuContent(this, &SUnrealAIAgentWindow::MakeModelPickerMenu)
				]

				// Spacer
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SSpacer)
				]

				// Mic / Action Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
					.ContentPadding(FMargin(6.0f, 4.0f))
					.ToolTipText(LOCTEXT("MicTooltip", "Voice / Context Action"))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("🎙️")))
						.Font(FAppStyle::GetFontStyle("Small"))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.62f, 0.68f)))
					]
				]

				// Circular Send Button ➔ / Stop Button ⏹
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					bIsRunning
					? SNew(SButton)
					  .ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.StopButton"))
					  .ContentPadding(FMargin(8.0f, 4.0f))
					  .ToolTipText(LOCTEXT("StopTooltip", "Stop Generation"))
					  .OnClicked(this, &SUnrealAIAgentWindow::OnCancelClicked)
					  [
						  SNew(STextBlock)
						  .Text(FText::FromString(TEXT("⏹")))
						  .Font(FAppStyle::GetFontStyle("SmallBold"))
						  .ColorAndOpacity(FSlateColor(FLinearColor::White))
					  ]
					: SNew(SButton)
					  .ButtonStyle(&AIStyle.GetWidgetStyle<FButtonStyle>("UnrealAIAgent.SendButton"))
					  .ContentPadding(FMargin(8.0f, 4.0f))
					  .ToolTipText(LOCTEXT("SendTooltip", "Send Prompt"))
					  .IsEnabled(this, &SUnrealAIAgentWindow::IsExecuteEnabled)
					  .OnClicked(this, &SUnrealAIAgentWindow::OnExecuteClicked)
					  [
						  SNew(STextBlock)
						  .Text(FText::FromString(TEXT("➔")))
						  .Font(FAppStyle::GetFontStyle("Bold"))
						  .ColorAndOpacity(FSlateColor(FLinearColor::White))
					  ]
				]
			]
		];
}

TSharedRef<SWidget> SUnrealAIAgentWindow::BuildFooterBar()
{
	return SNew(SHorizontalBox)
		// Left: Clickable Settings link
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FUnrealAIAgentStyle::Get().GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(4.0f, 2.0f))
			.OnClicked(this, &SUnrealAIAgentWindow::OnOpenSettingsClicked)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FooterSettings", "Antigravity - Settings"))
				.Font(FAppStyle::GetFontStyle("Small"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.48f, 0.5f, 0.55f)))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]

		// Right: Notification Bell Icon 🔔
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FUnrealAIAgentStyle::Get().GetWidgetStyle<FButtonStyle>("UnrealAIAgent.FlatIconButton"))
			.ContentPadding(FMargin(4.0f, 2.0f))
			.ToolTipText(this, &SUnrealAIAgentWindow::GetStatusText)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("🔔")))
				.Font(FAppStyle::GetFontStyle("Small"))
				.ColorAndOpacity(this, &SUnrealAIAgentWindow::GetStatusColor)
			]
		];
}

TSharedRef<SWidget> SUnrealAIAgentWindow::MakeModelPickerMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ModelGemini", "Gemini 3.7 Flash High"),
		LOCTEXT("ModelGeminiTip", "Fast, highly intelligent multimodal reasoning model."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::SetSelectedModel, FString(TEXT("gemini-3.7-flash-high"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ModelNemotron", "NVIDIA Nemotron 3 Ultra (550B)"),
		LOCTEXT("ModelNemotronTip", "Default high-capacity NVIDIA NIM model for complex Blueprint graph orchestration."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::SetSelectedModel, FString(TEXT("nvidia/nemotron-3-ultra-550b-a55b"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ModelLlama", "Meta Llama 3.3 (70B Instruct)"),
		LOCTEXT("ModelLlamaTip", "High performance general instruction-following model on NVIDIA NIM."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::SetSelectedModel, FString(TEXT("meta/llama-3.3-70b-instruct"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ModelMistral", "Mistral Large 2 (Instruct)"),
		LOCTEXT("ModelMistralTip", "Premier European multilingual and coding model on NVIDIA NIM."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::SetSelectedModel, FString(TEXT("mistralai/mistral-large-2-instruct"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ModelDeepSeek", "DeepSeek R1"),
		LOCTEXT("ModelDeepSeekTip", "Deep reasoning chain-of-thought model on NVIDIA NIM."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::SetSelectedModel, FString(TEXT("deepseek-ai/deepseek-r1"))))
	);

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SUnrealAIAgentWindow::MakePlusContextMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SugSword", "🗡️ Create Weapon Blueprint (BP_Sword)"),
		LOCTEXT("SugSwordTip", "Creates a weapon Blueprint with mesh, variables, and damage event."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::OnSuggestionClicked, FString(TEXT("Create a medieval sword Blueprint named BP_Sword in /Game/Weapons with 35 damage, static mesh component, and attack logic"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SugDoor", "🚪 Create Sliding Door Blueprint (BP_Door)"),
		LOCTEXT("SugDoorTip", "Creates an interactable sliding door Blueprint with collision & interact function."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::OnSuggestionClicked, FString(TEXT("Create an interactable sliding door Blueprint named BP_SlidingDoor in /Game/Props with BoxCollision and Interact function"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SugStats", "⚙️ Create Player Health & Stamina System"),
		LOCTEXT("SugStatsTip", "Creates character stats Blueprint with MaxHealth, Health, MaxStamina, and Stamina."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::OnSuggestionClicked, FString(TEXT("Create a character stats Blueprint named BP_PlayerStats in /Game/Blueprints with MaxHealth, Health, MaxStamina, and Stamina variables"))))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SugInspect", "🔍 Inspect Project Info & Content Structure"),
		LOCTEXT("SugInspectTip", "Queries all assets in /Game and returns summary statistics."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::OnSuggestionClicked, FString(TEXT("Get project summary info and find all assets in /Game"))))
	);

	return MenuBuilder.MakeWidget();
}

void SUnrealAIAgentWindow::SetSelectedModel(FString InModelId)
{
	UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::GetMutable();
	if (Settings)
	{
		Settings->ModelName = InModelId;
		Settings->SaveConfig();
	}
}

static TArray<TSharedPtr<FAIChatSession>> GSavedChatSessions;

void SUnrealAIAgentWindow::SaveCurrentSession()
{
	if (TurnHistory.Num() == 0)
	{
		return;
	}

	if (!CurrentSession.IsValid())
	{
		CurrentSession = MakeShared<FAIChatSession>();
		CurrentSession->Title = TurnHistory[0]->UserPrompt;
		if (CurrentSession->Title.Len() > 34)
		{
			CurrentSession->Title = CurrentSession->Title.Left(31) + TEXT("...");
		}
		GSavedChatSessions.Add(CurrentSession);
	}

	CurrentSession->Turns = TurnHistory;
	CurrentSession->LLMHistory = FUnrealAIAgentOrchestrator::Get().GetConversationHistory();
	CurrentSession->Timestamp = FDateTime::Now();
}

void SUnrealAIAgentWindow::LoadSession(TSharedPtr<FAIChatSession> SessionToLoad)
{
	if (!SessionToLoad.IsValid())
	{
		return;
	}

	SaveCurrentSession();

	CurrentSession = SessionToLoad;
	TurnHistory = SessionToLoad->Turns;
	ActiveTurn.Reset();
	TurnWidgets.Empty();

	FUnrealAIAgentOrchestrator::Get().SetConversationHistory(SessionToLoad->LLMHistory);

	RebuildMainView();
}

void SUnrealAIAgentWindow::ClearAllSessions()
{
	GSavedChatSessions.Empty();
	CurrentSession.Reset();
	TurnHistory.Empty();
	ActiveTurn.Reset();
	TurnWidgets.Empty();
	FUnrealAIAgentOrchestrator::Get().ClearHistory();
	RebuildMainView();
}

TSharedRef<SWidget> SUnrealAIAgentWindow::MakeHistoryMenu()
{
	SaveCurrentSession();

	FMenuBuilder MenuBuilder(true, nullptr);

	if (GSavedChatSessions.Num() == 0)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("NoHistory", "No saved conversations yet"),
			LOCTEXT("NoHistoryTip", "Your conversations in this project will automatically appear here."),
			FSlateIcon(),
			FUIAction()
		);
	}
	else
	{
		for (int32 Index = GSavedChatSessions.Num() - 1; Index >= 0; --Index)
		{
			TSharedPtr<FAIChatSession> Session = GSavedChatSessions[Index];
			if (!Session.IsValid())
			{
				continue;
			}

			FString TimeStr = Session->Timestamp.ToString(TEXT("%H:%M"));
			FText Label = FText::FromString(FString::Printf(TEXT("💬 [%s] %s (%d turns)"), *TimeStr, *Session->Title, Session->Turns.Num()));
			FText ToolTip = FText::FromString(FString::Printf(TEXT("Resume chat session with %d turns (last active at %s)"), Session->Turns.Num(), *Session->Timestamp.ToString()));

			MenuBuilder.AddMenuEntry(
				Label,
				ToolTip,
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::LoadSession, Session))
			);
		}

		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ClearAllHist", "🗑️ Clear All Chat History"),
			LOCTEXT("ClearAllHistTip", "Delete all saved conversations and start fresh."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SUnrealAIAgentWindow::ClearAllSessions))
		);
	}

	return MenuBuilder.MakeWidget();
}


FReply SUnrealAIAgentWindow::OnNewChatClicked()
{
	SaveCurrentSession();
	CurrentSession.Reset();
	TurnHistory.Empty();
	ActiveTurn.Reset();
	TurnWidgets.Empty();
	FUnrealAIAgentOrchestrator::Get().ClearHistory();
	RebuildMainView();
	return FReply::Handled();
}

FReply SUnrealAIAgentWindow::OnMoreOptionsClicked()
{
	return OnOpenSettingsClicked();
}

FReply SUnrealAIAgentWindow::OnOpenSettingsClicked()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "Plugins", "Unreal AI Agent");
	return FReply::Handled();
}

FReply SUnrealAIAgentWindow::OnPlusActionsClicked()
{
	return FReply::Handled();
}

void SUnrealAIAgentWindow::OnSuggestionClicked(FString PromptText)
{
	if (InputTextBox.IsValid())
	{
		InputTextBox->SetText(FText::FromString(PromptText));
	}
}


FReply SUnrealAIAgentWindow::OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if (!InKeyEvent.IsShiftDown() && !InKeyEvent.IsControlDown())
		{
			return OnExecuteClicked();
		}
	}
	return FReply::Unhandled();
}

FText SUnrealAIAgentWindow::GetHeaderTitleText() const
{
	if (TurnHistory.Num() > 0 && TurnHistory.Last().IsValid())
	{
		FString Prompt = TurnHistory.Last()->UserPrompt;
		if (Prompt.Len() > 28)
		{
			Prompt = Prompt.Left(25) + TEXT("...");
		}
		return FText::FromString(Prompt);
	}
	return LOCTEXT("DefaultHeaderTitle", "Agent");
}

FText SUnrealAIAgentWindow::GetActiveModelDisplayName() const
{
	const UUnrealAIAgentSettings* Settings = UUnrealAIAgentSettings::Get();
	FString Model = Settings ? Settings->ModelName : TEXT("gemini-3.7-flash-high");

	if (Model.Contains(TEXT("gemini")))
	{
		return FText::FromString(TEXT("Gemini 3.7 Flash High"));
	}
	if (Model.Contains(TEXT("nemotron")))
	{
		return FText::FromString(TEXT("Nemotron 3 Ultra"));
	}
	if (Model.Contains(TEXT("llama")))
	{
		return FText::FromString(TEXT("Llama 3.3 70B"));
	}
	if (Model.Contains(TEXT("mistral")))
	{
		return FText::FromString(TEXT("Mistral Large 2"));
	}
	if (Model.Contains(TEXT("deepseek")))
	{
		return FText::FromString(TEXT("DeepSeek R1"));
	}

	return FText::FromString(Model);
}

FText SUnrealAIAgentWindow::GetStatusText() const
{
	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	switch (Status)
	{
	case EUnrealAIAgentStatus::Idle:
		return LOCTEXT("StatusIdle", "Idle - Ready");
	case EUnrealAIAgentStatus::Planning:
		return LOCTEXT("StatusPlanning", "Planning...");
	case EUnrealAIAgentStatus::ExecutingTool:
		return LOCTEXT("StatusExecuting", "Executing Tool...");
	case EUnrealAIAgentStatus::Compiling:
		return LOCTEXT("StatusCompiling", "Compiling...");
	case EUnrealAIAgentStatus::Repairing:
		return LOCTEXT("StatusRepairing", "Self-Correcting...");
	case EUnrealAIAgentStatus::Completed:
		return LOCTEXT("StatusCompleted", "Completed");
	case EUnrealAIAgentStatus::Failed:
		return LOCTEXT("StatusFailed", "Failed");
	default:
		return LOCTEXT("StatusUnknown", "Unknown");
	}
}

FSlateColor SUnrealAIAgentWindow::GetStatusColor() const
{
	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	switch (Status)
	{
	case EUnrealAIAgentStatus::Idle:
		return FSlateColor(FLinearColor(0.5f, 0.55f, 0.6f));
	case EUnrealAIAgentStatus::Planning:
	case EUnrealAIAgentStatus::ExecutingTool:
	case EUnrealAIAgentStatus::Compiling:
	case EUnrealAIAgentStatus::Repairing:
		return FSlateColor(FLinearColor(0.0f, 0.52f, 1.0f));
	case EUnrealAIAgentStatus::Completed:
		return FSlateColor(FLinearColor(0.18f, 0.8f, 0.44f));
	case EUnrealAIAgentStatus::Failed:
		return FSlateColor(FLinearColor(0.9f, 0.22f, 0.21f));
	default:
		return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
	}
}

EVisibility SUnrealAIAgentWindow::GetLiveSpinnerVisibility() const
{
	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	if (Status != EUnrealAIAgentStatus::Idle && Status != EUnrealAIAgentStatus::Completed && Status != EUnrealAIAgentStatus::Failed)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

bool SUnrealAIAgentWindow::IsExecuteEnabled() const
{
	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	return (Status == EUnrealAIAgentStatus::Idle || Status == EUnrealAIAgentStatus::Completed || Status == EUnrealAIAgentStatus::Failed);
}

bool SUnrealAIAgentWindow::IsCancelEnabled() const
{
	EUnrealAIAgentStatus Status = FUnrealAIAgentOrchestrator::Get().GetStatus();
	return (Status != EUnrealAIAgentStatus::Idle && Status != EUnrealAIAgentStatus::Completed && Status != EUnrealAIAgentStatus::Failed);
}

void SUnrealAIAgentWindow::RefreshActiveTurnWidget()
{
	if (TurnWidgets.Num() > 0 && TurnWidgets.Last().IsValid())
	{
		TurnWidgets.Last()->RefreshContent();
	}
	if (ActivityScrollBox.IsValid())
	{
		ActivityScrollBox->ScrollToEnd();
	}
}

void SUnrealAIAgentWindow::OnStatusChanged(EUnrealAIAgentStatus NewStatus)
{
	if (ActiveTurn.IsValid())
	{
		ActiveTurn->Status = NewStatus;
		RefreshActiveTurnWidget();
	}
}

void SUnrealAIAgentWindow::OnActivity(const FString& Title, const FString& Detail)
{
	if (!ActiveTurn.IsValid())
	{
		return;
	}

	if (Title.Equals(TEXT("AI Thinking")))
	{
		ActiveTurn->ThinkingText = Detail;
	}
	else if (Title.Equals(TEXT("AI Plan")) || Title.Equals(TEXT("AI Reasoning")))
	{
		ActiveTurn->PlanText = Detail;
	}
	else if (Title.StartsWith(TEXT("Tool:")))
	{
		ActiveTurn->LiveStatusText = Detail;
	}

	RefreshActiveTurnWidget();
}

void SUnrealAIAgentWindow::OnToolExecuted(const FAIToolCall& ToolCall, const FAIToolResult& Result)
{
	if (!ActiveTurn.IsValid())
	{
		return;
	}

	FAIToolStepData Step;
	Step.ToolName = ToolCall.Name;
	Step.Summary = Result.bSuccess ? FString::Printf(TEXT("Executed %s"), *ToolCall.Name) : Result.ErrorMessage;
	Step.ToolArgsJson = ToolCall.ArgumentsJson;
	Step.ToolResultJson = Result.ReturnJson;
	Step.bSuccess = Result.bSuccess;

	ActiveTurn->ToolSteps.Add(Step);
	ActiveTurn->LiveStatusText = FString::Printf(TEXT("Executed %s"), *ToolCall.Name);

	RefreshActiveTurnWidget();
}

void SUnrealAIAgentWindow::OnCompleted(bool bSuccess, const FString& FinalSummary)
{
	if (ActiveTurn.IsValid())
	{
		ActiveTurn->bIsCompleted = true;
		ActiveTurn->bSuccess = bSuccess;
		ActiveTurn->FinalSummary = FinalSummary;
		ActiveTurn->Status = bSuccess ? EUnrealAIAgentStatus::Completed : EUnrealAIAgentStatus::Failed;
		RefreshActiveTurnWidget();
	}
	SaveCurrentSession();
	// Rebuild input bar button state
	RebuildMainView();
}


FReply SUnrealAIAgentWindow::OnExecuteClicked()
{
	if (!InputTextBox.IsValid())
	{
		return FReply::Handled();
	}

	FString PromptText = InputTextBox->GetText().ToString().TrimStartAndEnd();
	if (PromptText.IsEmpty())
	{
		return FReply::Handled();
	}

	InputTextBox->SetText(FText::GetEmpty());

	// Create new Turn
	ActiveTurn = MakeShared<FAITurnData>();
	ActiveTurn->UserPrompt = PromptText;
	ActiveTurn->Status = EUnrealAIAgentStatus::Planning;
	ActiveTurn->LiveStatusText = TEXT("Reasoning & Planning...");
	ActiveTurn->Timestamp = FDateTime::Now();

	TurnHistory.Add(ActiveTurn);

	// Rebuild view to show Active Chat state (Image 3)
	RebuildMainView();

	FUnrealAIAgentOrchestrator::Get().ExecuteUserRequest(PromptText);
	return FReply::Handled();
}

FReply SUnrealAIAgentWindow::OnCancelClicked()
{
	FUnrealAIAgentOrchestrator::Get().CancelExecution();
	if (ActiveTurn.IsValid())
	{
		ActiveTurn->Status = EUnrealAIAgentStatus::Failed;
		ActiveTurn->FinalSummary = TEXT("Execution cancelled by user.");
		ActiveTurn->bIsCompleted = true;
		ActiveTurn->bSuccess = false;
		RefreshActiveTurnWidget();
	}
	RebuildMainView();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
