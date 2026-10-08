// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Defeat sequence: screen fades to black -> white DEFEAT title -> run statistics -> respawn at the start point.

#include "RRHUD.h"
#include "RRGame.h"
#include "RRGameMode.h"
#include "Engine/Canvas.h"
#include "Internationalization/Text.h"

namespace
{
	float Ease(float T) { T = FMath::Clamp(T, 0.0f, 1.0f); return T * T * (3.0f - 2.0f * T); }
	float Fade(double T, double Start, double Dur) { return Ease((float)((T - Start) / FMath::Max(0.0001, Dur))); }
	FString Num(int32 V) { return FText::AsNumber(V).ToString(); }
	FString Clock(float Seconds)
	{
		const int32 S = FMath::Max(0, FMath::FloorToInt(Seconds));
		return FString::Printf(TEXT("%02d:%02d"), S / 60, S % 60);
	}

	// timeline (seconds after the player's health reaches zero)
	constexpr double TFadeStart = 0.9;
	constexpr double TFadeDur = 1.5;
	constexpr double TTitle = 2.5;
	constexpr double TStats = 4.2;
	constexpr double TRowStep = 0.42;
	constexpr double TEarned = TStats + 5 * TRowStep + 0.35;
	constexpr double TRespawnBar = TEarned + 1.6;
	constexpr double TRespawnDur = 4.2;
}

void ARRHUD::DebugShowDefeat()
{
	bFrontEndInit = true;
	if (Screen != ERRScreen::Playing) Screen = ERRScreen::Playing;
	bDebugDefeat = true;
	bDefeatActive = true;
	bDefeatRespawned = false;
	DefeatStart = FPlatformTime::Seconds();
}

void ARRHUD::UpdateDefeatState()
{
	ARRGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ARRGameMode>() : nullptr;
	if (GM)
	{
		const int32 State = (int32)GM->GetMatchState();
		if (State != PrevMatchState)
		{
			if (State == (int32)ERRMatchState::Lost)
			{
				bDebugDefeat = false;
				bDefeatActive = true;
				bDefeatRespawned = false;
				DefeatStart = Now;
				if (bInventoryOpen) { CloseInventory(); }
			}
			PrevMatchState = State;
		}
	}

	if (bDefeatActive && bDefeatRespawned && (Now - RespawnFadeStart) > FadeInDuration)
	{
		bDefeatActive = false;
		bDebugDefeat = false;
	}
}

void ARRHUD::DrawDefeat()
{
	if (bDefeatRespawned) return; // the shared fade-in (DrawFade) takes over

	ARRGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ARRGameMode>() : nullptr;
	FRRRunStats S = GM ? GM->GetRunStats() : FRRRunStats();
	if (bDebugDefeat && S.Kills == 0 && S.DamageDealt <= 0.0f)
	{
		S.DamageDealt = 842.0f; S.DamageTaken = 100.0f; S.Kills = 2; S.TimeSeconds = 74.0f; S.Scrap = 521; S.SessionScrap = 521; S.WeaponName = TEXT("Dark Sword II");
	}

	const double T = Now - DefeatStart;
	const float CX = CanvasW * 0.5f;

	// 1) the whole screen fades to black
	const float Black = Fade(T, TFadeStart, TFadeDur);
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.0f, 0.0f, 0.0f, Black));
	if (T < TTitle - 0.3) return;
	Vignette(0.5f * Black);

	// 2) DEFEAT: white letters that settle from wide tracking
	const float TitleIn = Fade(T, TTitle, 1.5);
	const float TitleY = CanvasH * 0.085f;
	const float Tracking = FMath::Lerp(80.0f, 28.0f, TitleIn);
	const FString Title = TEXT("DEFEAT");
	// faint red afterglow behind the white
	for (int32 g = 1; g <= 3; ++g)
	{
		Txt(Title, CX, TitleY, 176.0f + g * 3.0f, FLinearColor(0.9f, 0.08f, 0.1f, 0.07f * TitleIn), 0.5f, Tracking, false);
	}
	Txt(Title, CX, TitleY, 176.0f, FLinearColor(1.0f, 1.0f, 1.0f, TitleIn), 0.5f, Tracking);

	const float RuleW = U(620.0f) * Fade(T, TTitle + 0.7, 1.0);
	const float RuleY = TitleY + U(205.0f);
	Box(CX - RuleW * 0.5f, RuleY, RuleW, FMath::Max(2.0f, U(2.5f)), FLinearColor(0.88f, 0.12f, 0.14f, TitleIn));
	Txt(TEXT("CHASSIS OFFLINE"), CX, RuleY + U(18.0f), 24.0f, FLinearColor(0.62f, 0.65f, 0.7f, Fade(T, TTitle + 1.0, 1.0)), 0.5f, 14.0f);

	// 3) statistics, row by row
	const float ColW = U(780.0f);
	const float LX = CX - ColW * 0.5f;
	const float RX = CX + ColW * 0.5f;
	const float RowH = U(54.0f);
	float Y = RuleY + U(84.0f);

	struct FRow { FString Label; FString Value; double Start; float Numeric; bool bNumber; };
	const FRow Rows[5] = {
		{ TEXT("DAMAGE DEALT"), Num(FMath::RoundToInt(S.DamageDealt)), TStats + 0 * TRowStep, S.DamageDealt, true },
		{ TEXT("ENEMIES DEFEATED"), Num(S.Kills), TStats + 1 * TRowStep, (float)S.Kills, true },
		{ TEXT("DAMAGE TAKEN"), Num(FMath::RoundToInt(S.DamageTaken)), TStats + 2 * TRowStep, S.DamageTaken, true },
		{ TEXT("TIME SURVIVED"), Clock(S.TimeSeconds), TStats + 3 * TRowStep, 0.0f, false },
		{ TEXT("WEAPON"), S.WeaponName.IsEmpty() ? FString(TEXT("-")) : S.WeaponName.ToUpper(), TStats + 4 * TRowStep, 0.0f, false },
	};
	for (int32 i = 0; i < 5; ++i)
	{
		const FRow& R = Rows[i];
		const float A = Fade(T, R.Start, 0.45);
		if (A <= 0.0f) { Y += RowH; continue; }
		const float Slide = (1.0f - A) * U(34.0f);
		FString Val = R.Value;
		if (R.bNumber)
		{
			const float Count = Fade(T, R.Start + 0.1, 1.1);
			Val = Num(FMath::RoundToInt(R.Numeric * Count));
		}
		Txt(R.Label, LX - Slide, Y, 26.0f, FLinearColor(0.58f, 0.61f, 0.66f, A), 0.0f, 5.0f);
		Txt(Val, RX + Slide, Y - U(2.0f), 30.0f, FLinearColor(1.0f, 1.0f, 1.0f, A), 1.0f, 2.0f);
		Box(LX, Y + RowH - U(10.0f), ColW * A, 1.0f, FLinearColor(1, 1, 1, 0.14f * A));
		Y += RowH;
	}

	// 4) what was earned
	const float EA = Fade(T, TEarned, 0.7);
	if (EA > 0.0f)
	{
		const float BoxY = Y + U(14.0f);
		const float BoxH = U(112.0f);
		HGrad(LX, BoxY, ColW, BoxH, FLinearColor(1.0f, 0.62f, 0.16f, 0.16f * EA), FLinearColor(1.0f, 0.62f, 0.16f, 0.02f * EA), 24);
		Box(LX, BoxY, U(5.0f), BoxH, FLinearColor(1.0f, 0.62f, 0.16f, EA));
		Txt(TEXT("SCRAP EARNED"), LX + U(30.0f), BoxY + U(18.0f), 24.0f, FLinearColor(1.0f, 0.78f, 0.45f, EA), 0.0f, 6.0f);
		Txt(FString::Printf(TEXT("SESSION TOTAL  %s"), *Num(S.SessionScrap)), LX + U(30.0f), BoxY + U(66.0f), 20.0f, FLinearColor(0.7f, 0.65f, 0.55f, EA), 0.0f, 3.0f);
		const float Count = Fade(T, TEarned + 0.2, 1.3);
		Txt(FString::Printf(TEXT("+ %s"), *Num(FMath::RoundToInt(S.Scrap * Count))), RX - U(24.0f), BoxY + U(14.0f), 78.0f, FLinearColor(1.0f, 0.68f, 0.2f, EA), 1.0f, 3.0f);
	}

	// 5) respawn countdown (Enter / click skips)
	const double TR = T - TRespawnBar;
	if (TR > 0.0)
	{
		const float RA = Fade(TR, 0.0, 0.6);
		const float BarY = CanvasH - U(112.0f);
		const float BarW = U(520.0f);
		const float Prog = FMath::Clamp((float)(TR / TRespawnDur), 0.0f, 1.0f);
		Txt(TEXT("RESTORING CHASSIS AT START POINT"), CX, BarY - U(46.0f), 24.0f, FLinearColor(0.85f, 0.88f, 0.92f, RA), 0.5f, 6.0f);
		Box(CX - BarW * 0.5f, BarY, BarW, FMath::Max(2.0f, U(3.0f)), FLinearColor(1, 1, 1, 0.14f * RA));
		Box(CX - BarW * 0.5f, BarY, BarW * Prog, FMath::Max(2.0f, U(3.0f)), FLinearColor(0.95f, 0.95f, 0.95f, RA));
		const float Pulse = 0.55f + 0.45f * FMath::Sin((float)Now * 4.0f);
		Txt(TEXT("ENTER  -  RESPAWN NOW"), CX, BarY + U(20.0f), 20.0f, FLinearColor(0.55f, 0.58f, 0.62f, RA * Pulse), 0.5f, 5.0f);

		const bool bSkip = (bConfirm || bClickL) && T > TStats + 2.0;
		if (Prog >= 1.0f || bSkip)
		{
			if (GM && !bDebugDefeat)
			{
				GM->RespawnPlayer();
			}
			bDefeatRespawned = true;
			RespawnFadeStart = Now;
			FadeInStart = Now;
			FadeInDuration = 1.8f;
			UE_LOG(LogRR, Log, TEXT("Defeat screen finished -> respawn"));
		}
	}
}
