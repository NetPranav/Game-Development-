// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UnrealAIAgentStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateBrush.h"
#include "Styling/AppStyle.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUnrealAIAgentStyle::StyleInstance = nullptr;

void FUnrealAIAgentStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUnrealAIAgentStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

FName FUnrealAIAgentStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UnrealAIAgentStyle"));
	return StyleSetName;
}

const ISlateStyle& FUnrealAIAgentStyle::Get()
{
	return *StyleInstance;
}

void FUnrealAIAgentStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<FSlateStyleSet> FUnrealAIAgentStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("UnrealAIAgentStyle"));
	
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("UnrealAIAgent");
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	Style->Set("UnrealAIAgent.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("Icon128"), FVector2D(20.0f, 20.0f)));

	// Obsidian Theme Colors (True sRGB Hex #151515)
	const FLinearColor ColorObsidianBg = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("151515")));        // #151515 deep dark background
	const FLinearColor ColorCardBg = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1E1F23")));            // #1E1F23 card background
	const FLinearColor ColorUserBubbleBg = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("212328")));        // #212328 user bubble
	const FLinearColor ColorChipBg = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1A1B1E")));              // #1A1B1E chip/pill
	const FLinearColor ColorChipHover = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("282A30")));            // #282A30

	const FLinearColor ColorBlueNormal = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0084FF")));             // #0084FF
	const FLinearColor ColorBlueHover = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1A91FF")));               // #1A91FF
	const FLinearColor ColorBluePressed = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("006FD6")));
	const FLinearColor ColorRedStop = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E53935")));                // #E53935
	const FLinearColor ColorRedStopHover = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF4D4D")));

	// Card, Bubble, and Chip Brushes with Smooth Rounded Corners
	const FLinearColor ColorCardBorder = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2A2C32")));
	const FLinearColor ColorUserBubbleBorder = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2E3038")));
	const FLinearColor ColorChipBorder = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("26282E")));
	const FLinearColor ColorChipBorderHover = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("3A3C44")));

	Style->Set("UnrealAIAgent.WindowBg", new FSlateColorBrush(ColorObsidianBg));
	Style->Set("UnrealAIAgent.CardBg", new FSlateRoundedBoxBrush(ColorCardBg, 10.0f, ColorCardBorder, 1.0f));
	Style->Set("UnrealAIAgent.UserBubbleBg", new FSlateRoundedBoxBrush(ColorUserBubbleBg, 10.0f, ColorUserBubbleBorder, 1.0f));
	Style->Set("UnrealAIAgent.ChipBg", new FSlateRoundedBoxBrush(ColorChipBg, 6.0f, ColorChipBorder, 1.0f));
	Style->Set("UnrealAIAgent.ChipBgHover", new FSlateRoundedBoxBrush(ColorChipHover, 6.0f, ColorChipBorderHover, 1.0f));

	// Input Text Box Style (Completely Transparent inside card)
	FEditableTextBoxStyle InputBoxStyle = FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	InputBoxStyle.SetBackgroundImageNormal(FSlateColorBrush(FLinearColor::Transparent));
	InputBoxStyle.SetBackgroundImageHovered(FSlateColorBrush(FLinearColor::Transparent));
	InputBoxStyle.SetBackgroundImageFocused(FSlateColorBrush(FLinearColor::Transparent));
	InputBoxStyle.SetBackgroundImageReadOnly(FSlateColorBrush(FLinearColor::Transparent));
	InputBoxStyle.SetBackgroundColor(FSlateColor(FLinearColor::Transparent));
	Style->Set("UnrealAIAgent.InputBox", InputBoxStyle);

	// Send Button Style (Circular Blue Button with 14px radius)
	FButtonStyle SendButtonStyle = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(ColorBlueNormal, 14.0f))
		.SetHovered(FSlateRoundedBoxBrush(ColorBlueHover, 14.0f))
		.SetPressed(FSlateRoundedBoxBrush(ColorBluePressed, 14.0f))
		.SetDisabled(FSlateRoundedBoxBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("303338"))), 14.0f));
	Style->Set("UnrealAIAgent.SendButton", SendButtonStyle);

	// Stop Button Style (Circular Red Button with 14px radius)
	FButtonStyle StopButtonStyle = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(ColorRedStop, 14.0f))
		.SetHovered(FSlateRoundedBoxBrush(ColorRedStopHover, 14.0f))
		.SetPressed(FSlateRoundedBoxBrush(ColorRedStop * 0.8f, 14.0f))
		.SetDisabled(FSlateRoundedBoxBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("303338"))), 14.0f));
	Style->Set("UnrealAIAgent.StopButton", StopButtonStyle);

	// Flat Icon Button Style (for Top Header Icons +, 🕒, ..., ✕)
	FButtonStyle FlatIconButtonStyle = FButtonStyle()
		.SetNormal(FSlateColorBrush(FLinearColor::Transparent))
		.SetHovered(FSlateRoundedBoxBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("26282E"))), 4.0f))
		.SetPressed(FSlateRoundedBoxBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("202126"))), 4.0f))
		.SetDisabled(FSlateColorBrush(FLinearColor::Transparent));
	Style->Set("UnrealAIAgent.FlatIconButton", FlatIconButtonStyle);

	// Model Pill Button Style (6px rounded capsule)
	FButtonStyle ModelPillStyle = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(ColorChipBg, 6.0f, ColorChipBorder, 1.0f))
		.SetHovered(FSlateRoundedBoxBrush(ColorChipHover, 6.0f, ColorChipBorderHover, 1.0f))
		.SetPressed(FSlateRoundedBoxBrush(ColorChipBg, 6.0f, ColorBlueNormal, 1.0f))
		.SetDisabled(FSlateRoundedBoxBrush(ColorChipBg, 6.0f, ColorChipBorder, 1.0f));
	Style->Set("UnrealAIAgent.ModelPill", ModelPillStyle);

	return Style;
}

#undef RootToContentDir



