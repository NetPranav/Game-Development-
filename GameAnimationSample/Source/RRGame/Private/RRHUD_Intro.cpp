// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Intro film (Content/Movies/RR_Intro.mp4, optional), screen flashes, and the arena entry prompt.

#include "RRHUD.h"
#include "RRGame.h"
#include "RRGameMode.h"
#include "RRArena.h"
#include "MediaPlayer.h"
#include "FileMediaSource.h"
#include "MediaTexture.h"
#include "MediaSoundComponent.h"
#include "Engine/Canvas.h"
#include "Misc/Paths.h"

namespace
{
	FString In_IntroPath() { return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Movies/RR_Intro.mp4")); }
}

bool ARRHUD::StartIntroVideo()
{
	const FString Path = In_IntroPath();
	if (!FPaths::FileExists(Path))
	{
		UE_LOG(LogRR, Log, TEXT("Intro video not found (%s): skipping"), *Path);
		return false;
	}

	IntroPlayer = NewObject<UMediaPlayer>(this);
	IntroPlayer->PlayOnOpen = true;
	IntroPlayer->SetLooping(false);
	IntroSource = NewObject<UFileMediaSource>(this);
	IntroSource->SetFilePath(Path);
	IntroTexture = NewObject<UMediaTexture>(this);
	IntroTexture->SetMediaPlayer(IntroPlayer);
	IntroTexture->UpdateResource();
	IntroSound = NewObject<UMediaSoundComponent>(this);
	IntroSound->SetMediaPlayer(IntroPlayer);
	IntroSound->RegisterComponent();
	IntroSound->Activate();

	if (!IntroPlayer->OpenSource(IntroSource))
	{
		UE_LOG(LogRR, Warning, TEXT("Intro video could not be opened: %s"), *Path);
		EndIntroVideo();
		return false;
	}
	IntroStart = FPlatformTime::Seconds();
	bIntroWasPlaying = false;
	UE_LOG(LogRR, Log, TEXT("Intro video playing: %s"), *Path);
	return true;
}

void ARRHUD::DrawIntroVideo()
{
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor::Black);
	const double T = Now - IntroStart;

	if (IntroTexture && IntroPlayer && IntroPlayer->IsPlaying())
	{
		bIntroWasPlaying = true;
		const float VW = FMath::Max(1.0f, (float)IntroTexture->GetWidth());
		const float VH = FMath::Max(1.0f, (float)IntroTexture->GetHeight());
		const float S = FMath::Min(CanvasW / VW, CanvasH / VH);
		const float W = VW * S, H = VH * S;
		FCanvasTileItem Tile(FVector2D((CanvasW - W) * 0.5f, (CanvasH - H) * 0.5f), IntroTexture->GetResource(), FVector2D(W, H), FLinearColor::White);
		Tile.BlendMode = SE_BLEND_Opaque;
		Canvas->DrawItem(Tile);
	}

	const float Pulse = 0.45f + 0.35f * FMath::Sin((float)Now * 3.0f);
	Txt(TEXT("ENTER  -  SKIP"), CanvasW - U(48.0f), CanvasH - U(56.0f), 18.0f, FLinearColor(1, 1, 1, Pulse), 1.0f, 5.0f);

	const bool bSkip = (bConfirm || bBack || bClickL) && T > 0.5;
	const bool bEnded = bIntroWasPlaying && IntroPlayer && !IntroPlayer->IsPlaying() && T > 1.0;
	const bool bFailed = !bIntroWasPlaying && T > 6.0;
	if (bSkip || bEnded || bFailed || T > 180.0)
	{
		EndIntroVideo();
		FinishLoading();   // Screen is IntroVideo, so this now starts the game
	}
}

void ARRHUD::EndIntroVideo()
{
	if (IntroPlayer) { IntroPlayer->Close(); }
	if (IntroSound) { IntroSound->Deactivate(); IntroSound->DestroyComponent(); }
	IntroPlayer = nullptr;
	IntroSource = nullptr;
	IntroTexture = nullptr;
	IntroSound = nullptr;
}

void ARRHUD::Flash(const FLinearColor& Color, float Duration)
{
	FlashColor = Color;
	FlashStart = FPlatformTime::Seconds();
	FlashDuration = FMath::Max(0.05f, Duration);
}

void ARRHUD::DrawFlash()
{
	const double T = Now - FlashStart;
	if (T < 0.0 || T > FlashDuration) return;
	const float K = 1.0f - (float)(T / FlashDuration);
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(FlashColor.R, FlashColor.G, FlashColor.B, K * K * 0.9f));
}

void ARRHUD::DrawArenaPrompt()
{
	ARRGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ARRGameMode>() : nullptr;
	ARRArena* Arena = GM ? GM->GetArena() : nullptr;
	if (!Arena || bDefeatActive || !GM->CanEnterArena()) return;
	if (!Arena->IsPlayerNearEntry() && Arena->GetEntryProgress() <= 0.0f) return;

	const float P = Arena->GetEntryProgress();
	const float CX = CanvasW * 0.5f;
	const float Y = CanvasH * 0.70f;

	// progress ring (24 segments) around a diamond
	const float R = U(34.0f);
	const int32 Seg = 32;
	for (int32 i = 0; i < Seg; ++i)
	{
		const float A0 = -PI * 0.5f + 2.0f * PI * i / Seg;
		const float A1 = -PI * 0.5f + 2.0f * PI * (i + 1) / Seg;
		const bool bLit = (float)i / Seg < P;
		const FLinearColor C = bLit ? FLinearColor(1.0f, 0.95f, 0.8f, 1.0f) : FLinearColor(1, 1, 1, 0.22f);
		Line(CX + FMath::Cos(A0) * R, Y + FMath::Sin(A0) * R, CX + FMath::Cos(A1) * R, Y + FMath::Sin(A1) * R, FMath::Max(2.0f, U(3.0f)), C);
	}
	RotRect(CX, Y, U(14.0f), U(14.0f), 45.0f, FLinearColor(0.35f, 0.8f, 1.0f, 0.6f + 0.4f * P));

	const FString Title = P > 0.0f ? TEXT("ENTERING THE ARENA") : TEXT("STAND IN THE CIRCLE TO ENTER THE ARENA");
	Txt(Title, CX, Y + U(52.0f), 22.0f, FLinearColor(1, 1, 1, 0.92f), 0.5f, 6.0f);
	if (ARRBoss* Boss = GM->FindBoss())
	{
		Txt(Boss->GetBossName().ToString().ToUpper() + TEXT("  AWAITS"), CX, Y + U(86.0f), 16.0f, FLinearColor(0.85f, 0.25f, 0.2f, 0.85f), 0.5f, 8.0f);
	}
}
