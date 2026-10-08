// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Intro splash, start menu (with Blockchain + Settings panels) and the GTA-style loading screen.

#include "RRHUD.h"
#include "RRGame.h"
#include "RRGameMode.h"
#include "RRSuiBridge.h"
#include "AudioDevice.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"

namespace
{
	float Ease(float T) { T = FMath::Clamp(T, 0.0f, 1.0f); return T * T * (3.0f - 2.0f * T); }
	float Fade(double T, double Start, double Dur) { return Ease((float)((T - Start) / FMath::Max(0.0001, Dur))); }

	const FLinearColor Amber(1.0f, 0.62f, 0.16f, 1.0f);
	const FLinearColor Steel(0.86f, 0.89f, 0.92f, 1.0f);
	const FLinearColor Dim(0.55f, 0.58f, 0.62f, 1.0f);

	FLinearColor WithA(FLinearColor C, float A) { C.A = A; return C; }
}

void ARRHUD::EnterUIMode(bool bCursor, bool bPause)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(bCursor);
		if (bPause) PC->SetPause(true);
	}
}

void ARRHUD::LeaveUIMode()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->SetPause(false);
	}
}

void ARRHUD::SetScreen(ERRScreen NewScreen)
{
	auto IsMenuFamily = [](ERRScreen S) { return S == ERRScreen::Menu || S == ERRScreen::Settings || S == ERRScreen::Blockchain; };
	const ERRScreen Old = Screen;
	Screen = NewScreen;
	if (!(IsMenuFamily(Old) && IsMenuFamily(NewScreen)))
	{
		ScreenStart = FPlatformTime::Seconds();
	}
	SubSel = 0;
	UE_LOG(LogRR, Log, TEXT("HUD screen -> %d"), (int32)NewScreen);

	switch (NewScreen)
	{
	case ERRScreen::Splash:
		EnterUIMode(false, /*bPause*/ false); // world timers must keep running so the player finishes setting up
		break;
	case ERRScreen::Menu:
		if (!IsMenuFamily(Old)) { MenuSel = 0; EnterUIMode(true); }
		break;
	case ERRScreen::Loading:
		LoadStart = FPlatformTime::Seconds();
		LoadProgress = 0.0f;
		bLoadingExit = false;
		EnterUIMode(false, /*bPause*/ false); // keep whatever pause state the menu had; never freeze timers before the player is ready
		break;
	case ERRScreen::Playing:
		if (Old != ERRScreen::Playing) { LeaveUIMode(); }
		break;
	default:
		break;
	}
}

FString ARRHUD::ShortAddress(const FString& Addr) const
{
	if (Addr.Len() <= 14) return Addr;
	return Addr.Left(8) + TEXT("...") + Addr.Right(6);
}

void ARRHUD::StartNewGame()
{
	SetScreen(ERRScreen::Loading);
}

void ARRHUD::FinishLoading()
{
	if (ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>())
	{
		GM->BeginGameplay();
	}
	if (GEngine) GEngine->bEnableOnScreenDebugMessages = true;
	SetScreen(ERRScreen::Playing);
	FadeInStart = FPlatformTime::Seconds();
	FadeInDuration = 1.6f;
}

// ---------------------------------------------------------------------------------------------
// Splash: black -> logo -> KAIZEEN -> quote -> fade out
// ---------------------------------------------------------------------------------------------
void ARRHUD::DrawSplash()
{
	const double T = Now - ScreenStart;
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor::Black);

	const double TotalHold = 7.6;
	const double OutDur = 1.1;
	if ((T > 0.9) && (bConfirm || bClickL || bBack) && T < TotalHold)
	{
		ScreenStart -= (TotalHold - T); // skip ahead to the fade-out
		return;
	}

	const float Out = 1.0f - Fade(T, TotalHold, OutDur);
	const float CX = CanvasW * 0.5f;
	const float CY = CanvasH * 0.5f;

	Embers(0.32f * Fade(T, 1.2, 2.0) * Out);

	// emblem
	const float LogoA = Fade(T, 0.7, 1.6) * Out;
	const float LogoSize = U(150.0f) * (0.94f + 0.06f * Ease((float)((T - 0.7) / 2.0)));
	DrawLogoMark(CX, CY - U(205.0f), LogoSize, LogoA);

	// title: letters reveal one by one while the tracking settles
	const FString Title = TEXT("KAIZEEN");
	const float TrackT = Ease((float)((T - 1.8) / 3.2));
	const float Tracking = FMath::Lerp(64.0f, 26.0f, TrackT);
	const float TitleSize = 168.0f;
	const float TitleW = TxtWidth(Title, TitleSize, Tracking);
	float PX = CX - TitleW * 0.5f;
	const float TY = CY - U(110.0f);
	for (int32 i = 0; i < Title.Len(); ++i)
	{
		const FString Ch = Title.Mid(i, 1);
		const float A = Fade(T, 1.8 + i * 0.17, 0.7) * Out;
		const float Lift = (1.0f - Fade(T, 1.8 + i * 0.17, 0.9)) * U(18.0f);
		Txt(Ch, PX, TY + Lift, TitleSize, FLinearColor(0.95f, 0.96f, 0.98f, A), 0.0f, 0.0f);
		PX += TxtWidth(Ch, TitleSize) + U(Tracking);
	}

	// hairline
	const float LineA = Fade(T, 3.9, 1.0) * Out;
	const float LineW = U(560.0f) * Fade(T, 3.9, 1.3);
	Box(CX - LineW * 0.5f, CY + U(88.0f), LineW, FMath::Max(1.0f, U(2.0f)), WithA(Amber, LineA));

	// quote
	const float QA = Fade(T, 4.6, 1.5) * Out;
	Txt(TEXT("Every fall teaches. Every blade remembers."), CX, CY + U(118.0f), 34.0f, FLinearColor(0.72f, 0.75f, 0.80f, QA), 0.5f, 3.0f);
	Txt(TEXT("KAIZEEN   //   CONTINUOUS IMPROVEMENT"), CX, CY + U(176.0f), 20.0f, FLinearColor(0.45f, 0.48f, 0.52f, QA * 0.9f), 0.5f, 7.0f);

	Vignette(0.55f * Out);

	if (T >= TotalHold + OutDur)
	{
		SetScreen(ERRScreen::Menu);
		ScreenStart = Now;
	}
}

// ---------------------------------------------------------------------------------------------
// Start menu
// ---------------------------------------------------------------------------------------------
void ARRHUD::DrawMenu()
{
	// Pause only once the player pawn is fully set up (loadout / robot visual are timer driven).
	if (APlayerController* PC0 = GetOwningPlayerController())
	{
		ARRGameMode* GM0 = GetWorld()->GetAuthGameMode<ARRGameMode>();
		if (!PC0->IsPaused() && (!GM0 || GM0->IsPlayerReady()))
		{
			PC0->SetPause(true);
		}
	}
	const double T = Now - ScreenStart;
	const float In = Fade(T, 0.0, 1.4);

	// --- backdrop: slow Ken Burns on the key art, with a cinematic grade ---
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.01f, 0.012f, 0.016f, 1.0f));
	if (MenuBackground)
	{
		const float Zoom = 1.10f + 0.035f * FMath::Sin((float)T * 0.11f);
		Cover(MenuBackground, In, Zoom, FMath::Sin((float)T * 0.06f) * 0.8f, 0.0f, 0.0f, 0.0f, CanvasW, CanvasH);
	}
	else
	{
		VGrad(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.05f, 0.06f, 0.08f, 1.0f), FLinearColor(0.01f, 0.01f, 0.015f, 1.0f));
	}
	HGrad(0.0f, 0.0f, CanvasW * 0.62f, CanvasH, FLinearColor(0.0f, 0.0f, 0.0f, 0.90f * In), FLinearColor(0.0f, 0.0f, 0.0f, 0.0f), 40);
	VGrad(0.0f, CanvasH * 0.72f, CanvasW, CanvasH * 0.28f, FLinearColor(0, 0, 0, 0), FLinearColor(0, 0, 0, 0.78f * In), 24);
	Vignette(0.55f * In);
	Embers(0.85f * In);

	const float ML = CanvasW * 0.075f;

	// --- title block ---
	DrawLogoMark(ML + U(34.0f), CanvasH * 0.135f, U(68.0f), In);
	Txt(TEXT("KAIZEEN"), ML + U(92.0f), CanvasH * 0.085f, 118.0f, FLinearColor(0.96f, 0.97f, 0.99f, In), 0.0f, 20.0f);
	Txt(TEXT("ROBOT   RUMBLE"), ML + U(96.0f), CanvasH * 0.085f + U(124.0f), 28.0f, WithA(Amber, In), 0.0f, 16.0f);
	Box(ML + U(96.0f), CanvasH * 0.085f + U(168.0f), U(420.0f) * Fade(T, 0.5, 1.2), FMath::Max(1.0f, U(2.0f)), FLinearColor(1, 1, 1, 0.35f * In));

	// --- menu list ---
	static const TCHAR* Items[] = { TEXT("CONTINUE"), TEXT("NEW GAME"), TEXT("BLOCKCHAIN"), TEXT("SETTINGS"), TEXT("QUIT") };
	static const TCHAR* Hints[] = {
		TEXT("Pick up where you left off in the arena"),
		TEXT("Start a fresh run"),
		TEXT("Connect your Sui wallet and manage on-chain gear"),
		TEXT("Display, performance and audio"),
		TEXT("Leave the arena") };
	const int32 NumItems = 5;
	const float ItemH = U(76.0f);
	const float ListY = CanvasH * 0.40f;
	const float ListW = U(560.0f);
	const bool bTop = (Screen == ERRScreen::Menu);

	if (bTop)
	{
		if (bUp) MenuSel = (MenuSel + NumItems - 1) % NumItems;
		if (bDown) MenuSel = (MenuSel + 1) % NumItems;
	}

	int32 Activated = -1;
	for (int32 i = 0; i < NumItems; ++i)
	{
		const float Y = ListY + i * ItemH;
		const float Stagger = Fade(T, 0.35 + i * 0.09, 0.7);
		if (bTop && Hit(ML, Y, ListW, ItemH))
		{
			if (MenuSel != i) { MenuSel = i; }
			if (bClickL) { Activated = i; bClickL = false; }
		}
		const bool bSel = (MenuSel == i);
		const float Slide = (1.0f - Stagger) * U(-40.0f) + (bSel ? U(14.0f) : 0.0f);
		const float A = Stagger * (bTop ? 1.0f : 0.45f);

		if (bSel)
		{
			HGrad(ML - U(14.0f), Y + U(6.0f), ListW, ItemH - U(12.0f), FLinearColor(1.0f, 0.62f, 0.16f, 0.20f * A), FLinearColor(1.0f, 0.62f, 0.16f, 0.0f), 24);
			Box(ML - U(14.0f), Y + U(6.0f), U(5.0f), ItemH - U(12.0f), WithA(Amber, A));
		}
		const FLinearColor Col = bSel ? FLinearColor(1.0f, 1.0f, 1.0f, A) : FLinearColor(0.62f, 0.65f, 0.69f, A * 0.9f);
		Txt(Items[i], ML + Slide, Y + U(12.0f), 46.0f, Col, 0.0f, 5.0f);
	}
	if (bTop)
	{
		Txt(Hints[MenuSel], ML, ListY + NumItems * ItemH + U(22.0f), 24.0f, FLinearColor(0.70f, 0.73f, 0.78f, In * 0.9f), 0.0f, 1.5f);
		if (bConfirm) Activated = MenuSel;
	}

	// --- activation ---
	if (Activated >= 0)
	{
		switch (Activated)
		{
		case 0:
		case 1: StartNewGame(); return;
		case 2: SetScreen(ERRScreen::Blockchain); break;
		case 3: SetScreen(ERRScreen::Settings); break;
		case 4:
			if (APlayerController* PC = GetOwningPlayerController()) { PC->ConsoleCommand(TEXT("quit")); }
			break;
		}
	}

	// --- side panels ---
	const float PX = CanvasW * 0.50f, PY = CanvasH * 0.17f, PW = CanvasW * 0.43f, PH = CanvasH * 0.66f;
	if (Screen == ERRScreen::Settings) DrawSettingsPanel(PX, PY, PW, PH);
	else if (Screen == ERRScreen::Blockchain) DrawBlockchainPanel(PX, PY, PW, PH);

	// --- footer ---
	URRSuiBridge* Sui = GetGameInstance() ? GetGameInstance()->GetSubsystem<URRSuiBridge>() : nullptr;
	const bool bConn = Sui && Sui->IsConnected();
	const float FY = CanvasH - U(64.0f);
	Box(ML, FY + U(6.0f), U(12.0f), U(12.0f), bConn ? FLinearColor(0.2f, 0.9f, 0.5f, In) : FLinearColor(0.85f, 0.25f, 0.25f, In));
	Txt(bConn ? TEXT("SUI TESTNET  -  WALLET CONNECTED") : TEXT("SUI TESTNET  -  WALLET NOT CONNECTED"), ML + U(26.0f), FY, 22.0f, FLinearColor(0.75f, 0.78f, 0.82f, In), 0.0f, 3.0f);
	Txt(TEXT("W S / UP DOWN  SELECT        ENTER  CONFIRM        ESC  BACK"), CanvasW - ML, FY, 22.0f, FLinearColor(0.55f, 0.58f, 0.62f, In), 1.0f, 3.0f);
	Txt(TEXT("KAIZEEN  -  PROTOTYPE BUILD"), CanvasW - ML, FY - U(34.0f), 18.0f, FLinearColor(0.45f, 0.48f, 0.52f, In), 1.0f, 3.0f);

	if (Screen != ERRScreen::Menu && bBack)
	{
		SetScreen(ERRScreen::Menu);
	}

}

void ARRHUD::DrawSettingsPanel(float PX, float PY, float PW, float PH)
{
	const double T = Now - ScreenStart;
	const float In = Fade(T, 0.0, 0.5);
	Box(PX, PY, PW, PH, FLinearColor(0.02f, 0.025f, 0.03f, 0.80f * In));
	Frame(PX, PY, PW, PH, FMath::Max(1.0f, U(1.5f)), FLinearColor(0.7f, 0.74f, 0.8f, 0.35f * In));
	Brackets(PX - U(6.0f), PY - U(6.0f), PW + U(12.0f), PH + U(12.0f), U(26.0f), FMath::Max(2.0f, U(2.5f)), WithA(Amber, In));
	Txt(TEXT("SETTINGS"), PX + U(40.0f), PY + U(30.0f), 44.0f, FLinearColor(1, 1, 1, In), 0.0f, 8.0f);
	Box(PX + U(40.0f), PY + U(92.0f), PW - U(80.0f), 1.0f, FLinearColor(1, 1, 1, 0.25f * In));

	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	IConsoleVariable* CVarSP = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	if (CVarSP && ResScale > 99.5f) { ResScale = FMath::Clamp(CVarSP->GetFloat(), 50.0f, 100.0f); }

	static const TCHAR* Modes[] = { TEXT("FULLSCREEN"), TEXT("BORDERLESS"), TEXT("WINDOWED") };
	static const TCHAR* Quals[] = { TEXT("LOW"), TEXT("MEDIUM"), TEXT("HIGH"), TEXT("EPIC") };

	int32 ModeIdx = 2, Qual = 2;
	bool bVSync = false;
	if (GUS)
	{
		const EWindowMode::Type M = GUS->GetFullscreenMode();
		ModeIdx = (M == EWindowMode::Fullscreen) ? 0 : (M == EWindowMode::WindowedFullscreen ? 1 : 2);
		Qual = FMath::Clamp(GUS->GetOverallScalabilityLevel(), 0, 3);
		bVSync = GUS->IsVSyncEnabled();
	}

	const int32 Rows = 6;
	if (bUp) SubSel = (SubSel + Rows - 1) % Rows;
	if (bDown) SubSel = (SubSel + 1) % Rows;

	const float RowH = U(82.0f);
	const float Y0 = PY + U(116.0f);
	int32 Delta = 0;
	if (bLeft) Delta = -1;
	if (bRight) Delta = 1;
	bool bActivate = bConfirm;

	for (int32 i = 0; i < Rows; ++i)
	{
		const float Y = Y0 + i * RowH;
		FString Label, Value;
		switch (i)
		{
		case 0: Label = TEXT("DISPLAY MODE"); Value = Modes[ModeIdx]; break;
		case 1: Label = TEXT("VSYNC"); Value = bVSync ? TEXT("ON") : TEXT("OFF"); break;
		case 2: Label = TEXT("QUALITY"); Value = Quals[Qual]; break;
		case 3: Label = TEXT("RESOLUTION SCALE"); Value = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(ResScale)); break;
		case 4: Label = TEXT("MASTER VOLUME"); Value = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.0f)); break;
		default: Label = TEXT("BACK"); break;
		}

		const bool bHover = Hit(PX + U(20.0f), Y, PW - U(40.0f), RowH - U(8.0f));
		int32 RowDelta = 0;
		if (bHover)
		{
			SubSel = i;
			if (bClickL)
			{
				bClickL = false;
				if (i == Rows - 1) bActivate = true;
				else RowDelta = (MouseX < PX + PW * 0.70f) ? -1 : 1;
			}
		}
		const bool bSel = (SubSel == i);
		if (bSel)
		{
			HGrad(PX + U(20.0f), Y, PW - U(40.0f), RowH - U(8.0f), FLinearColor(1.0f, 0.62f, 0.16f, 0.16f * In), FLinearColor(1.0f, 0.62f, 0.16f, 0.0f), 20);
			Box(PX + U(20.0f), Y, U(4.0f), RowH - U(8.0f), WithA(Amber, In));
		}
		const FLinearColor C = bSel ? FLinearColor(1, 1, 1, In) : FLinearColor(0.66f, 0.69f, 0.73f, In);
		Txt(Label, PX + U(48.0f), Y + U(16.0f), 32.0f, C, 0.0f, 3.0f);
		if (i < Rows - 1)
		{
			Txt(FString::Printf(TEXT("<  %s  >"), *Value), PX + PW - U(48.0f), Y + U(16.0f), 32.0f, bSel ? WithA(Amber, In) : C, 1.0f, 2.0f);
		}

		int32 D = (bSel ? Delta : 0) + RowDelta;
		if (i == Rows - 1)
		{
			if (bSel && bActivate) { SetScreen(ERRScreen::Menu); return; }
			continue;
		}
		if (bSel && bActivate && Delta == 0 && RowDelta == 0) D = 1; // Enter cycles forward
		if (D == 0) continue;

		switch (i)
		{
		case 0:
			if (GUS && !GIsEditor)
			{
				const int32 N = (ModeIdx + D + 3) % 3;
				GUS->SetFullscreenMode(N == 0 ? EWindowMode::Fullscreen : (N == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed));
				GUS->ApplySettings(false);
			}
			break;
		case 1:
			if (GUS) { GUS->SetVSyncEnabled(!bVSync); GUS->ApplySettings(false); }
			break;
		case 2:
			if (GUS)
			{
				GUS->SetOverallScalabilityLevel(FMath::Clamp(Qual + D, 0, 3));
				GUS->ApplyNonResolutionSettings();
				GUS->SaveSettings();
			}
			break;
		case 3:
			ResScale = FMath::Clamp(ResScale + D * 10.0f, 50.0f, 100.0f);
			if (CVarSP) CVarSP->Set(ResScale, ECVF_SetByGameSetting);
			break;
		case 4:
			MasterVolume = FMath::Clamp(MasterVolume + D * 0.1f, 0.0f, 1.0f);
			if (FAudioDeviceHandle AD = GEngine->GetMainAudioDevice()) { AD->SetTransientPrimaryVolume(MasterVolume); }
			break;
		}
	}
}

void ARRHUD::DrawBlockchainPanel(float PX, float PY, float PW, float PH)
{
	const double T = Now - ScreenStart;
	const float In = Fade(T, 0.0, 0.5);
	Box(PX, PY, PW, PH, FLinearColor(0.02f, 0.025f, 0.03f, 0.80f * In));
	Frame(PX, PY, PW, PH, FMath::Max(1.0f, U(1.5f)), FLinearColor(0.7f, 0.74f, 0.8f, 0.35f * In));
	Brackets(PX - U(6.0f), PY - U(6.0f), PW + U(12.0f), PH + U(12.0f), U(26.0f), FMath::Max(2.0f, U(2.5f)), WithA(Amber, In));
	Txt(TEXT("BLOCKCHAIN"), PX + U(40.0f), PY + U(30.0f), 44.0f, FLinearColor(1, 1, 1, In), 0.0f, 8.0f);
	Box(PX + U(40.0f), PY + U(92.0f), PW - U(80.0f), 1.0f, FLinearColor(1, 1, 1, 0.25f * In));

	URRSuiBridge* Sui = GetGameInstance() ? GetGameInstance()->GetSubsystem<URRSuiBridge>() : nullptr;
	const bool bConn = Sui && Sui->IsConnected();

	// info block
	float Y = PY + U(116.0f);
	auto Info = [&](const TCHAR* Key, const FString& Val, const FLinearColor& ValCol)
	{
		Txt(Key, PX + U(48.0f), Y, 22.0f, FLinearColor(0.55f, 0.58f, 0.62f, In), 0.0f, 4.0f);
		Txt(Val, PX + PW - U(48.0f), Y - U(3.0f), 28.0f, ValCol, 1.0f, 1.5f);
		Y += U(44.0f);
	};
	Info(TEXT("NETWORK"), TEXT("SUI TESTNET"), FLinearColor(0.80f, 0.85f, 0.95f, In));
	Info(TEXT("STATUS"), bConn ? TEXT("CONNECTED") : TEXT("NOT CONNECTED"), bConn ? FLinearColor(0.3f, 0.95f, 0.55f, In) : FLinearColor(0.95f, 0.38f, 0.35f, In));
	Info(TEXT("WALLET"), bConn ? ShortAddress(Sui->GetWalletAddress()) : FString(TEXT("-")), FLinearColor(0.85f, 0.88f, 0.92f, In));
	Info(TEXT("CREDITS"), bConn ? FString::FromInt(Sui->GetCredits()) : FString(TEXT("-")), WithA(Amber, In));
	Info(TEXT("GEAR NFTS"), bConn ? FString::FromInt(Sui->GetOwnedGear().Num()) : FString(TEXT("-")), FLinearColor(0.45f, 0.85f, 1.0f, In));

	// actions
	struct FAct { const TCHAR* Label; };
	static const FAct Acts[] = { {TEXT("CONNECT WALLET")}, {TEXT("REFRESH GEAR")}, {TEXT("VIEW ON SUISCAN")}, {TEXT("GET TESTNET SUI")}, {TEXT("BACK")} };
	const int32 Rows = 5;
	if (bUp) SubSel = (SubSel + Rows - 1) % Rows;
	if (bDown) SubSel = (SubSel + 1) % Rows;

	const float RowH = U(60.0f);
	const float Y0 = Y + U(14.0f);
	for (int32 i = 0; i < Rows; ++i)
	{
		const float RY = Y0 + i * RowH;
		bool bActivate = false;
		if (Hit(PX + U(20.0f), RY, PW - U(40.0f), RowH - U(6.0f)))
		{
			SubSel = i;
			if (bClickL) { bActivate = true; bClickL = false; }
		}
		const bool bSel = (SubSel == i);
		if (bSel && bConfirm) bActivate = true;
		if (bSel)
		{
			HGrad(PX + U(20.0f), RY, PW - U(40.0f), RowH - U(6.0f), FLinearColor(1.0f, 0.62f, 0.16f, 0.16f * In), FLinearColor(1.0f, 0.62f, 0.16f, 0.0f), 20);
			Box(PX + U(20.0f), RY, U(4.0f), RowH - U(6.0f), WithA(Amber, In));
		}
		const bool bDisabled = (i == 2 && !bConn);
		FLinearColor C = bSel ? FLinearColor(1, 1, 1, In) : FLinearColor(0.66f, 0.69f, 0.73f, In);
		if (bDisabled) C = FLinearColor(0.35f, 0.37f, 0.4f, In);
		Txt(Acts[i].Label, PX + U(48.0f), RY + U(11.0f), 30.0f, C, 0.0f, 3.0f);

		if (!bActivate || bDisabled) continue;
		switch (i)
		{
		case 0: if (Sui) Sui->BeginWalletConnect(); break;
		case 1: if (Sui) Sui->FetchGear(); break;
		case 2: if (Sui) FPlatformProcess::LaunchURL(*(FString(TEXT("https://suiscan.xyz/testnet/account/")) + Sui->GetWalletAddress()), nullptr, nullptr); break;
		case 3: FPlatformProcess::LaunchURL(TEXT("https://faucet.sui.io"), nullptr, nullptr); break;
		case 4: SetScreen(ERRScreen::Menu); return;
		}
	}
	Txt(TEXT("Wallet connect opens a local browser page. Gear NFTs you own on testnet appear in your inventory."), PX + PW * 0.5f, PY + PH - U(46.0f), 18.0f, FLinearColor(0.5f, 0.53f, 0.57f, In), 0.5f, 1.0f);
}

// ---------------------------------------------------------------------------------------------
// Loading screen: crossfading key art with a black bar at the bottom (tips + progress)
// ---------------------------------------------------------------------------------------------
void ARRHUD::DrawLoading()
{
	const double T = Now - LoadStart;
	const float BarH = CanvasH * 0.165f;
	const float ImgH = CanvasH - BarH;

	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor::Black);

	// slideshow
	const int32 N = LoadingImages.Num();
	const double SlideLen = 3.6;
	if (N > 0)
	{
		const int32 Idx = (int32)FMath::FloorToDouble(T / SlideLen);
		const float Phase = (float)(T / SlideLen - Idx);
		auto DrawSlide = [&](int32 Slide, float Alpha, float Local)
		{
			UTexture2D* Tex = LoadingImages[Slide % N];
			const float Zoom = 1.04f + 0.12f * Local;
			const float Dir = (Slide % 2 == 0) ? 1.0f : -1.0f;
			Cover(Tex, Alpha, Zoom, Dir * (-0.5f + Local), 0.15f, 0.0f, 0.0f, CanvasW, ImgH);
		};
		DrawSlide(Idx, 1.0f, Phase);
		if (Phase > 0.78f)
		{
			DrawSlide(Idx + 1, Ease((Phase - 0.78f) / 0.22f), 0.0f);
		}
	}
	else
	{
		VGrad(0.0f, 0.0f, CanvasW, ImgH, FLinearColor(0.05f, 0.06f, 0.08f, 1.0f), FLinearColor(0.01f, 0.01f, 0.015f, 1.0f));
	}
	// cinematic grade: darken edges and the lower part of the picture so text reads
	Vignette(0.55f);
	VGrad(0.0f, ImgH * 0.55f, CanvasW, ImgH * 0.45f, FLinearColor(0, 0, 0, 0), FLinearColor(0, 0, 0, 0.62f), 24);
	Embers(0.5f);

	// chapter card on the picture
	const float CardA = Fade(T, 0.6, 1.0);
	Txt(TEXT("CHAPTER 01"), CanvasW * 0.06f, ImgH - U(122.0f), 24.0f, WithA(Amber, CardA), 0.0f, 8.0f);
	Txt(TEXT("THE ARENA"), CanvasW * 0.06f, ImgH - U(92.0f), 58.0f, FLinearColor(1, 1, 1, CardA), 0.0f, 10.0f);

	// the black bar
	Box(0.0f, ImgH, CanvasW, BarH + 1.0f, FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));

	// progress model: time-paced (so the art is seen), capped until the world reports ready
	ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
	const bool bReady = !GM || GM->IsPlayerReady();
	const float MinSeconds = 7.2f;
	const float Target = FMath::Min((float)(T / MinSeconds), bReady ? 1.0f : 0.92f);
	LoadProgress = FMath::Max(LoadProgress, Target);

	// track + fill along the top edge of the bar
	const float TrackH = FMath::Max(2.0f, U(3.0f));
	Box(0.0f, ImgH, CanvasW, TrackH, FLinearColor(1, 1, 1, 0.12f));
	const float FillW = CanvasW * LoadProgress;
	Box(0.0f, ImgH, FillW, TrackH, Amber);
	Box(FillW - U(90.0f), ImgH, U(90.0f), TrackH, FLinearColor(1.0f, 0.85f, 0.5f, 0.9f));
	HGrad(FMath::Max(0.0f, FillW - U(240.0f)), ImgH - U(10.0f), FMath::Min(U(240.0f), FillW), U(10.0f), FLinearColor(1.0f, 0.62f, 0.16f, 0.0f), FLinearColor(1.0f, 0.62f, 0.16f, 0.35f), 16);

	// bar content
	const float BX = CanvasW * 0.06f;
	const int32 Dots = (int32)(T * 2.5) % 4;
	Txt(FString(TEXT("LOADING")) + FString::ChrN(Dots, TEXT('.')), BX, ImgH + U(26.0f), 34.0f, FLinearColor(1, 1, 1, 1), 0.0f, 9.0f);

	static const TCHAR* Tips[] = {
		TEXT("Click once, twice, three times: the third strike is a finisher that staggers."),
		TEXT("Tap block just before a hit lands to parry and stagger your attacker."),
		TEXT("Dodge grants brief invulnerability in the middle of the roll."),
		TEXT("Press Q to lock on to the nearest enemy. [ and ] switch targets."),
		TEXT("Press Tab to manage your gear. Equip a different sword to change your damage."),
		TEXT("Connect a Sui wallet to mint new gear NFTs after you win a match."),
		TEXT("Heavy attacks (F) cost more stamina but break poise faster.") };
	const int32 NumTips = UE_ARRAY_COUNT(Tips);
	const double TipLen = 4.2;
	const int32 TipIdx = (int32)(T / TipLen) % NumTips;
	const float TipPhase = (float)(T / TipLen - FMath::FloorToDouble(T / TipLen));
	const float TipA = FMath::Min(Ease(TipPhase / 0.12f), 1.0f - Ease((TipPhase - 0.88f) / 0.12f));
	Txt(FString::Printf(TEXT("TIP   %s"), Tips[TipIdx]), BX, ImgH + U(80.0f), 24.0f, FLinearColor(0.72f, 0.75f, 0.80f, TipA), 0.0f, 1.5f);

	Txt(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(LoadProgress * 100.0f)), CanvasW - CanvasW * 0.06f, ImgH + U(26.0f), 34.0f, WithA(Amber, 1.0f), 1.0f, 4.0f);
	Txt(TEXT("KAIZEEN"), CanvasW - CanvasW * 0.06f, ImgH + U(80.0f), 24.0f, FLinearColor(0.5f, 0.53f, 0.57f, 1.0f), 1.0f, 12.0f);

	// fade in at start, fade out at the end
	const float StartA = 1.0f - Fade(T, 0.0, 0.9);
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0, 0, 0, StartA));

	if (!bLoadingExit && LoadProgress >= 0.985f && T > MinSeconds - 0.2f && bReady)
	{
		bLoadingExit = true;
		LoadingExitStart = Now;
	}
	if (bLoadingExit)
	{
		const float A = Fade(Now - LoadingExitStart, 0.0, 0.8);
		Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0, 0, 0, A));
		if (Now - LoadingExitStart >= 0.85)
		{
			FinishLoading();
		}
	}
}
