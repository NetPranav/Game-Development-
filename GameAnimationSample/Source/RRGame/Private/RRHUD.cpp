// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRHUD.h"
#include "RRGame.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerState.h"
#include "RRGameMode.h"
#include "RRSettings.h"
#include "RRLoadoutComponent.h"
#include "RRSuiBridge.h"
#include "RREnemyDirector.h"
#include "RREnemy.h"
#include "RRBoss.h"
#include "GameFramework/AAAAPlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AAAAttributeSet.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"

ARRHUD::ARRHUD()
{
}

void ARRHUD::BeginPlay()
{
	Super::BeginPlay();
	BindKeys();

	BossPhaseHandle = FRRBossEvents::OnPhaseChanged().AddUObject(this, &ARRHUD::OnBossPhaseChanged);
	BossPerilousHandle = FRRBossEvents::OnPerilousWarning().AddUObject(this, &ARRHUD::OnBossPerilousWarning);
}

void ARRHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FRRBossEvents::OnPhaseChanged().Remove(BossPhaseHandle);
	FRRBossEvents::OnPerilousWarning().Remove(BossPerilousHandle);
	Super::EndPlay(EndPlayReason);
}

void ARRHUD::BindKeys()
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->InputComponent)
	{
		PC->InputComponent->BindKey(EKeys::F2, IE_Pressed, this, &ARRHUD::OnKeyF2).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::F3, IE_Pressed, this, &ARRHUD::OnKeyF3).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ARRHUD::OnKey1).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ARRHUD::OnKey2).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ARRHUD::OnKey3).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ARRHUD::OnKey4).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ARRHUD::OnKey5).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ARRHUD::OnKey6).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ARRHUD::OnKey7).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ARRHUD::OnKey8).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ARRHUD::OnKey9).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ARRHUD::OnKeyEnter).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::C, IE_Pressed, this, &ARRHUD::OnKeyC).bExecuteWhenPaused = true;

		PC->InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ARRHUD::OnMouseScrollUp).bExecuteWhenPaused = true;
		PC->InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ARRHUD::OnMouseScrollDown).bExecuteWhenPaused = true;
	}
}

bool ARRHUD::ToggleGameMenu()
{
	// Intro / menus / loading keep their own pause + cursor: Tab and Esc must not undo it.
	if (Screen != ERRScreen::Playing)
	{
		return true;
	}
	// No inventory while the defeat sequence is running.
	if (bDefeatActive)
	{
		return false;
	}

	if (bInventoryOpen)
	{
		CloseInventory();
		return false;
	}

	bInventoryOpen = true;
	InvOpenTime = FPlatformTime::Seconds();
	InvSel = 0;
	InvScrollRow = 0;
	ActiveTab = 0;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}
	return true;
}

void ARRHUD::CloseInventory()
{
	bInventoryOpen = false;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bEnableClickEvents = false;
		PC->bEnableMouseOverEvents = false;
	}
}

void ARRHUD::OnMouseScrollUp()
{
	if (bInventoryOpen) InvScrollRow = FMath::Max(0, InvScrollRow - 1);
}

void ARRHUD::OnMouseScrollDown()
{
	if (bInventoryOpen) InvScrollRow++; // clamped while drawing
}

void ARRHUD::OnKeyF2()
{
	if (bInventoryOpen || Screen != ERRScreen::Playing) return;
	if (URRSuiBridge* Sui = GetGameInstance()->GetSubsystem<URRSuiBridge>())
	{
		Sui->BeginWalletConnect();
	}
}

void ARRHUD::OnKeyF3()
{
	if (bInventoryOpen || Screen != ERRScreen::Playing) return;
	if (URRSuiBridge* Sui = GetGameInstance()->GetSubsystem<URRSuiBridge>())
	{
		Sui->FetchGear();
	}
}

void ARRHUD::EquipGearIndex(int32 Index)
{
	// number keys pick gear only inside the inventory: in play, 1 / mouse wheel draw and holster the sword
	if (Screen != ERRScreen::Playing || !bInventoryOpen) return;
	APlayerController* PC = GetOwningPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (Pawn)
	{
		if (URRLoadoutComponent* Loadout = Pawn->FindComponentByClass<URRLoadoutComponent>())
		{
			TArray<FRRInventoryItem> Inv = Loadout->BuildInventory();
			if (Inv.IsValidIndex(Index))
			{
				Loadout->Equip(Inv[Index].Gear);
			}
		}
	}
}

void ARRHUD::OnKey1() { EquipGearIndex(0); }
void ARRHUD::OnKey2() { EquipGearIndex(1); }
void ARRHUD::OnKey3() { EquipGearIndex(2); }
void ARRHUD::OnKey4() { EquipGearIndex(3); }
void ARRHUD::OnKey5() { EquipGearIndex(4); }
void ARRHUD::OnKey6() { EquipGearIndex(5); }
void ARRHUD::OnKey7() { EquipGearIndex(6); }
void ARRHUD::OnKey8() { EquipGearIndex(7); }
void ARRHUD::OnKey9() { EquipGearIndex(8); }

void ARRHUD::OnKeyEnter()
{
	// Only used after a victory ("play again"); menus and the defeat screen poll Enter themselves.
	if (bInventoryOpen || bDefeatActive || Screen != ERRScreen::Playing) return;
	if (ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>())
	{
		if (GM->GetMatchState() == ERRMatchState::Won)
		{
			GM->RestartMatch();
		}
	}
}

void ARRHUD::OnKeyC()
{
	if (bInventoryOpen || Screen != ERRScreen::Playing) return;
	if (ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>())
	{
		FRRRewardResult Res = GM->GetLastReward();
		if (!Res.Url.IsEmpty())
		{
			FPlatformApplicationMisc::ClipboardCopy(*Res.Url);
		}
	}
}

void ARRHUD::DebugSetScreen(ERRScreen NewScreen)
{
	bFrontEndInit = true;
	SetScreen(NewScreen);
}

void ARRHUD::DebugOpenInventory()
{
	bFrontEndInit = true;
	if (Screen != ERRScreen::Playing)
	{
		Screen = ERRScreen::Playing;
	}
	if (!bInventoryOpen)
	{
		bInventoryOpen = true;
		InvOpenTime = FPlatformTime::Seconds();
		InvSel = 0;
		InvScrollRow = 0;
		if (APlayerController* PC2 = GetOwningPlayerController())
		{
			PC2->SetInputMode(FInputModeGameAndUI());
			PC2->SetShowMouseCursor(true);
			PC2->SetPause(true);
		}
	}
}

void ARRHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !GEngine) return;

	BeginFrame();

	if (!bFrontEndInit)
	{
		bFrontEndInit = true;
		ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
		if (GM && GM->IsFrontEndPending())
		{
			if (GEngine) GEngine->bEnableOnScreenDebugMessages = false; // keep the intro clean
			SetScreen(ERRScreen::Splash);
		}
		else
		{
			Screen = ERRScreen::Playing;
		}
	}

	// The project's dev overlay (on-screen debug lines) would draw over the styled screens.
	// Off during play too (cinematic presentation); "rr.debug.onscreen 1" brings the dev lines back.
	static TAutoConsoleVariable<int32> CVarRROnScreen(TEXT("rr.debug.onscreen"), 0, TEXT("1 = show the project's on-screen debug messages during play"));
	GEngine->bEnableOnScreenDebugMessages = CVarRROnScreen.GetValueOnGameThread() != 0 && (Screen == ERRScreen::Playing) && !bInventoryOpen && !bDefeatActive;

	switch (Screen)
	{
	case ERRScreen::Splash:
		DrawSplash();
		return;
	case ERRScreen::Menu:
	case ERRScreen::Settings:
	case ERRScreen::Blockchain:
		DrawMenu();
		DrawFade();
		return;
	case ERRScreen::Loading:
		DrawLoading();
		return;
	case ERRScreen::IntroVideo:
		DrawIntroVideo();
		return;
	default:
		break;
	}

	UpdateDefeatState();

	if (bInventoryOpen)
	{
		DrawInventory();
		return;
	}

	DrawDamageFx();
	DrawMatchHUD();
	DrawBossHud();
	DrawArenaPrompt();

	if (bDefeatActive)
	{
		DrawDefeat();
	}
	DrawFade();
	DrawFlash();
	DrawCinematicBars();
}

void ARRHUD::DrawFade()
{
	// Black -> clear after loading finishes / a respawn.
	const double T = Now - FadeInStart;
	if (T >= 0.0 && T < FadeInDuration)
	{
		const float A = 1.0f - FMath::Clamp((float)(T / FadeInDuration), 0.0f, 1.0f);
		Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.0f, 0.0f, 0.0f, A * A));
	}
}

void ARRHUD::DrawMatchHUD()
{
	UFont* MedFont = GEngine->GetMediumFont();
	UFont* SmallFont = GEngine->GetSmallFont();
	if (!MedFont || !SmallFont || !Canvas) return;

	APlayerController* PC = GetOwningPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
	URRSuiBridge* Sui = GetGameInstance()->GetSubsystem<URRSuiBridge>();
	URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>();

	// Player panel
	if (Pawn)
	{
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn)) ASC = ASI->GetAbilitySystemComponent();
		else if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState())) ASC = PS_ASI->GetAbilitySystemComponent();

		if (ASC)
		{
			float MaxS = ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxStaminaAttribute());
			float CurrS = ASC->GetNumericAttribute(UAAAAttributeSet::GetStaminaAttribute());
			float MaxP = ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxPoiseAttribute());
			float CurrP = ASC->GetNumericAttribute(UAAAAttributeSet::GetPoiseAttribute());

			float DT = GetWorld()->GetDeltaSeconds();
			DispStamina = FMath::Lerp(DispStamina, CurrS, 10.f * DT);
			DispPoise = FMath::Lerp(DispPoise, CurrP, 10.f * DT);

			GhostStamina = FMath::Lerp(GhostStamina, DispStamina, 3.f * DT);
			GhostPoise = FMath::Lerp(GhostPoise, DispPoise, 3.f * DT);

			if (GhostStamina < 1.0f && MaxS > 1.0f && CurrS > 1.0f) { GhostStamina = CurrS; DispStamina = CurrS; }
			if (GhostPoise < 1.0f && MaxP > 1.0f && CurrP > 1.0f) { GhostPoise = CurrP; DispPoise = CurrP; }

			// Stamina bar: 6 px high, 220 px wide, bottom-left
			if (MaxS > 0.0f)
			{
				float StaminaX = 50.0f;
				float StaminaY = Canvas->SizeY - 30.0f;
				DrawRect(FLinearColor(0,0,0,0.5f), StaminaX, StaminaY, 220.f, 6.f);
				DrawRect(FLinearColor(1,1,1,0.5f), StaminaX, StaminaY, (GhostStamina/MaxS)*220.f, 6.f);
				DrawRect(FLinearColor::Green, StaminaX, StaminaY, (DispStamina/MaxS)*220.f, 6.f);
			}

			// Posture bar: thin bar bottom-centre, 360 px wide, fills from centre outward with 1 - Poise/MaxPoise
			if (MaxP > 0.0f)
			{
				float DamageFrac = FMath::Clamp(1.0f - (DispPoise / MaxP), 0.0f, 1.0f);
				float PoiseW = 360.0f;
				float PoiseH = 4.0f;
				float CX = Canvas->SizeX / 2.0f;
				float CY = Canvas->SizeY - 40.0f;
				
				FLinearColor PoiseColor = FMath::Lerp(FLinearColor(1.0f, 0.4f, 0.0f), FLinearColor::Red, DamageFrac);
				if (DamageFrac > 0.9f)
				{
					if (FMath::Fmod(Now, 0.2) > 0.1)
					{
						PoiseColor = FLinearColor::White;
					}
				}
				
				float FillW = PoiseW * DamageFrac;
				DrawRect(FLinearColor(0,0,0,0.5f), CX - PoiseW/2.0f, CY, PoiseW, PoiseH);
				DrawRect(PoiseColor, CX - FillW/2.0f, CY, FillW, PoiseH);
			}
		}
	}

	// Enemy health bars
	if (Director && PC)
	{
		AAAAAPlayerController* AAAPC = Cast<AAAAAPlayerController>(PC);
		AActor* LockedTarget = AAAPC ? AAAPC->GetLockedTarget() : nullptr;

		TArray<ARREnemy*> Living = Director->GetLivingEnemies();
		FVector CamLoc; FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		for (ARREnemy* Enemy : Living)
		{
			if (!Enemy) continue;
			float Dist = FVector::Dist(CamLoc, Enemy->GetActorLocation());
			if (Dist > 2500.f) continue;

			FVector HeadLoc = Enemy->GetActorLocation() + FVector(0,0,100.f);
			FVector ScreenLoc = Project(HeadLoc);
			if (ScreenLoc.Z > 0.0f && !Enemy->IsA(ARRBoss::StaticClass()))
			{
				float HealthFrac = Enemy->GetHealthFraction();
				float HBarW = 50.f;
				float HBarH = 6.f;
				DrawRect(FLinearColor(0,0,0,0.5f), ScreenLoc.X - HBarW/2, ScreenLoc.Y, HBarW, HBarH);
				DrawRect(FLinearColor::Red, ScreenLoc.X - HBarW/2, ScreenLoc.Y, HBarW * HealthFrac, HBarH);

				if (UAbilitySystemComponent* EASC = Enemy->GetAbilitySystemComponent())
				{
					float MaxP = EASC->GetNumericAttribute(UAAAAttributeSet::GetMaxPoiseAttribute());
					float CurP = EASC->GetNumericAttribute(UAAAAttributeSet::GetPoiseAttribute());
					if (MaxP > 0)
					{
						DrawRect(FLinearColor::Yellow, ScreenLoc.X - HBarW/2, ScreenLoc.Y + HBarH, HBarW * (CurP/MaxP), 2.f);
					}
				}

				if (Enemy == LockedTarget)
				{
					DrawText(TEXT("[ ]"), FLinearColor::White, ScreenLoc.X - 10.f, ScreenLoc.Y - 15.f, MedFont);
				}
			}
		}
	}

	// Match status
	if (GM)
	{
		ERRMatchState State = GM->GetMatchState();
		FString StatusText;
		if (State == ERRMatchState::PreMatch) StatusText = TEXT("GET READY");
		else if (State == ERRMatchState::InProgress) StatusText = FString::Printf(TEXT("WAVES - %d REMAINING"), GM->GetEnemiesTotal() - GM->GetEnemiesKilled());

		const URRSettings* RRS = URRSettings::Get();
		const bool bArenaMode = GM->GetArena() != nullptr || (RRS && RRS->bBossMode);   // the boss bar owns the top
		if (bArenaMode) StatusText.Reset();
		if (!StatusText.IsEmpty() && State != ERRMatchState::Lost)
		{
			float TW, TH;
			GetTextSize(StatusText, TW, TH, MedFont);
			DrawText(StatusText, FLinearColor::White, (Canvas->SizeX - TW)/2.f, 20.f, MedFont);
		}

		if (State != ERRMatchState::Lost && !bArenaMode)
		{
			FString TimeText = FString::Printf(TEXT("TIME: %d"), FMath::FloorToInt(GM->GetMatchTimeSeconds()));
			float TTW, TTH; GetTextSize(TimeText, TTW, TTH, SmallFont);
			DrawText(TimeText, FLinearColor::White, (Canvas->SizeX - TTW)/2.f, 50.f, SmallFont);
		}

		if (State == ERRMatchState::Won)
		{
			FString Banner = TEXT("VICTORY");
			float BW, BH; GetTextSize(Banner, BW, BH, MedFont);
			DrawText(Banner, FLinearColor::White, (Canvas->SizeX - BW*2)/2.f, Canvas->SizeY/3.f, MedFont, 2.0f);

			FString Sub;
			if (GM->IsRewardPending()) Sub = TEXT("Minting reward on Sui...");
			else
			{
				FRRRewardResult Res = GM->GetLastReward();
				if (Res.bOk) Sub = FString::Printf(TEXT("Reward: %s  +%d CREDITS"), *Res.Minted.Name, Res.CreditsAdded);
				else Sub = Res.Error.IsEmpty() ? TEXT("Reward error") : Res.Error;
			}
			float SW, SH; GetTextSize(Sub, SW, SH, MedFont);
			DrawText(Sub, FLinearColor::Green, (Canvas->SizeX - SW)/2.f, Canvas->SizeY/3.f + 50.f, MedFont);

			FString Hint = (!GM->IsRewardPending() && GM->GetLastReward().bOk) ? TEXT("[C] copy link   [Enter] play again") : TEXT("[Enter] play again");
			float HW, HH; GetTextSize(Hint, HW, HH, SmallFont);
			DrawText(Hint, FLinearColor::White, (Canvas->SizeX - HW)/2.f, Canvas->SizeY/3.f + 100.f, SmallFont);
		}
	}

	// Wallet panel
	if (Sui)
	{
		float RX = Canvas->SizeX - 250.f;
		float RY = 20.f;
		if (Sui->IsConnected())
		{
			DrawText(FString::Printf(TEXT("Wallet: Connected %s"), *ShortAddress(Sui->GetWalletAddress())), FLinearColor::Green, RX, RY, SmallFont);
			RY += 20.f;
			DrawText(FString::Printf(TEXT("Credits: %d"), Sui->GetCredits()), FLinearColor::Yellow, RX, RY, SmallFont);
		}
		else
		{
			DrawText(TEXT("Press F2 to connect wallet"), FLinearColor::White, RX, RY, SmallFont);
		}
		RY += 30.f;

		if (Pawn)
		{
			if (URRLoadoutComponent* Loadout = Pawn->FindComponentByClass<URRLoadoutComponent>())
			{
				FRRGearData Wep, Arm, Ab;
				if (Loadout->GetEquipped(ERRGearKind::Weapon, Wep))
				{
					DrawText(FString::Printf(TEXT("Weapon  %s  DMG %d  Lv%d"), *Wep.Name, Wep.Damage, Wep.Level), FLinearColor::White, RX, RY, SmallFont);
					RY += 15.f;
				}
				if (Loadout->GetEquipped(ERRGearKind::Armor, Arm))
				{
					DrawText(FString::Printf(TEXT("Armor   %s  ARM %d  Lv%d"), *Arm.Name, Arm.Armor, Arm.Level), FLinearColor::White, RX, RY, SmallFont);
					RY += 15.f;
				}
				if (Loadout->GetEquipped(ERRGearKind::Ability, Ab))
				{
					DrawText(FString::Printf(TEXT("Ability %s  PWR %d  Lv%d"), *Ab.Name, Ab.Power, Ab.Level), FLinearColor::White, RX, RY, SmallFont);
					RY += 15.f;
				}
			}
		}
		DrawText(TEXT("[Tab] Inventory"), FLinearColor(0.6f, 0.6f, 0.6f), RX, RY + 6.f, SmallFont);
	}
}

// ---------------------------------------------------------------------------------------------
// Console helpers for UI testing / screenshots
// ---------------------------------------------------------------------------------------------
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "Engine/World.h"

namespace
{
	ARRHUD* FindRRHUD(UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? Cast<ARRHUD>(PC->GetHUD()) : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GRRUIScreenCmd(
		TEXT("rr.ui.screen"), TEXT("rr.ui.screen splash|menu|settings|chain|loading|playing"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ARRHUD* HUD = FindRRHUD(World);
			if (!HUD || Args.Num() < 1) return;
			const FString& A = Args[0];
			if (A == TEXT("splash")) HUD->DebugSetScreen(ERRScreen::Splash);
			else if (A == TEXT("menu")) HUD->DebugSetScreen(ERRScreen::Menu);
			else if (A == TEXT("settings")) HUD->DebugSetScreen(ERRScreen::Settings);
			else if (A == TEXT("chain")) HUD->DebugSetScreen(ERRScreen::Blockchain);
			else if (A == TEXT("loading")) HUD->DebugSetScreen(ERRScreen::Loading);
			else if (A == TEXT("playing")) HUD->DebugSetScreen(ERRScreen::Playing);
		}));

	FAutoConsoleCommandWithWorldAndArgs GRRUIDefeatCmd(
		TEXT("rr.ui.defeat"), TEXT("Plays the defeat screen with sample stats (no respawn)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const float Delay = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.0f;
			TWeakObjectPtr<UWorld> W(World);
			auto Run = [W]() { if (ARRHUD* HUD = FindRRHUD(W.Get())) HUD->DebugShowDefeat(); };
			if (Delay > 0.0f && World) { FTimerHandle H; World->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda(Run), Delay, false); } else { Run(); }
		}));

	FAutoConsoleCommandWithWorldAndArgs GRRUIInvCmd(
		TEXT("rr.ui.inventory"), TEXT("Opens the inventory screen"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const float Delay = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.0f;
			TWeakObjectPtr<UWorld> W(World);
			auto Run = [W]() { if (ARRHUD* HUD = FindRRHUD(W.Get())) HUD->DebugOpenInventory(); };
			if (Delay > 0.0f && World) { FTimerHandle H; World->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda(Run), Delay, false); } else { Run(); }
		}));

	FAutoConsoleCommandWithWorldAndArgs GRRFXHurtCmd(
		TEXT("rr.fx.hurt"), TEXT("rr.fx.hurt <fraction>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			float Frac = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.2f;
			if (ARRHUD* HUD = FindRRHUD(World)) HUD->DebugInjectHit(Frac);
		}));

	FAutoConsoleCommandWithWorldAndArgs GRRFXLowHealthCmd(
		TEXT("rr.fx.lowhealth"), TEXT("Toggles fake low health for UI testing"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (ARRHUD* HUD = FindRRHUD(World))
			{
				HUD->DebugToggleLowHealth();
			}
		}));
}
