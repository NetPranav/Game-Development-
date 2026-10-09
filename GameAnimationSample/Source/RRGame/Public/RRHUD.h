// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/AAAAHUD.h"
#include "RRTypes.h"
#include "RRBoss.h"
#include "RRHUD.generated.h"

class UTexture2D;
class UFont;
class URRLoadoutComponent;
struct FRRInventoryItem;

/** Which full-screen presentation the HUD is currently showing. */
UENUM()
enum class ERRScreen : uint8
{
	Splash,      // black screen: logo, KAIZEEN, quote
	Menu,        // start screen
	Settings,
	Blockchain,
	Loading,     // slideshow with a black bar at the bottom
	IntroVideo,  // optional Content/Movies/RR_Intro.mp4 after loading
	Playing
};

/**
 * Canvas-only HUD: in-match vitals, intro / start menu / loading screens, Resident-Evil style inventory and the defeat screen.
 * All of it is drawn with Canvas calls (no UMG / Slate brushes), laid out in a 1920x1080 reference space and scaled to the viewport.
 */
UCLASS()
class RRGAME_API ARRHUD : public AAAAAHUD
{
	GENERATED_BODY()

public:
	ARRHUD();

	virtual void DrawHUD() override;

	/** Test / debug hooks (rr.ui.* console commands). */
	void DebugSetScreen(ERRScreen NewScreen);
	void DebugShowDefeat();
	void DebugOpenInventory();
	void DebugInjectHit(float Fraction);
	void DebugToggleLowHealth() { DbgLowHealthFraction = (DbgLowHealthFraction < 0.0f) ? 0.15f : -1.0f; }

	ERRScreen GetScreen() const { return Screen; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ToggleGameMenu() override;

	// ---- shared drawing helpers (RRHUD_Draw.cpp) ----
	void BeginFrame();
	float U(float V) const { return V * UIScale; }
	UFont* BodyFont() const;
	void Txt(const FString& Text, float X, float Y, float Size, const FLinearColor& Color, float Align = 0.0f, float Tracking = 0.0f, bool bShadow = true);
	float TxtWidth(const FString& Text, float Size, float Tracking = 0.0f);
	void Box(float X, float Y, float W, float H, const FLinearColor& C);
	void Frame(float X, float Y, float W, float H, float Thick, const FLinearColor& C);
	void Brackets(float X, float Y, float W, float H, float Len, float Thick, const FLinearColor& C);
	void VGrad(float X, float Y, float W, float H, const FLinearColor& Top, const FLinearColor& Bottom, int32 Steps = 32);
	void HGrad(float X, float Y, float W, float H, const FLinearColor& Left, const FLinearColor& Right, int32 Steps = 32);
	void Vignette(float Strength);
	void Cover(UTexture2D* Tex, float Alpha, float Zoom, float PanX, float PanY, float X, float Y, float W, float H);
	void Line(float X0, float Y0, float X1, float Y1, float Thick, const FLinearColor& C);
	void Tri(FVector2D A, FVector2D B, FVector2D C, const FLinearColor& Col);
	void RotRect(float CX, float CY, float W, float H, float AngleDeg, const FLinearColor& Col);
	void Embers(float Alpha);
	void DrawLogoMark(float CX, float CY, float Size, float Alpha);
	void LoadUIAssets();

	// ---- input polling (works while paused) ----
	void PollInput();
	bool Hit(float X, float Y, float W, float H) const;
	bool KeyPressed(const FKey& Key) const { return PressedKeys.Contains(Key); }
	TSet<FKey> PressedKeys;
	TMap<FKey, bool> PrevKeyDown;
	float MouseX = 0.0f, MouseY = 0.0f;
	float LastMouseX = -1.0f, LastMouseY = -1.0f;
	bool bClickL = false, bClickR = false;
	bool bPrevL = false, bPrevR = false;
	bool bUp = false, bDown = false, bLeft = false, bRight = false, bConfirm = false, bBack = false;

	// ---- front end (RRHUD_FrontEnd.cpp) ----
	void SetScreen(ERRScreen NewScreen);
	void EnterUIMode(bool bCursor, bool bPause = true);
	void LeaveUIMode();
	void DrawSplash();
	void DrawMenu();
	void DrawSettingsPanel(float PX, float PY, float PW, float PH);
	void DrawBlockchainPanel(float PX, float PY, float PW, float PH);
	void DrawLoading();
	void StartNewGame();
	void FinishLoading();
	void DrawFade();
	FString ShortAddress(const FString& Addr) const;

	ERRScreen Screen = ERRScreen::Playing;
	bool bFrontEndInit = false;
	double ScreenStart = 0.0;
	double LoadStart = 0.0;
	int32 MenuSel = 1;
	int32 SubSel = 0;
	float LoadProgress = 0.0f;
	float MenuIntro = 0.0f;
	bool bLoadingExit = false;
	double LoadingExitStart = 0.0;
	double FadeInStart = -100.0;
	float FadeInDuration = 1.2f;
	float MasterVolume = 1.0f;
	float ResScale = 100.0f;

	// ---- inventory (RRHUD_Inventory.cpp) ----
	void DrawInventory();
	void DrawItemIcon(const FRRInventoryItem& Item, float X, float Y, float S, float Alpha);
	void DrawGearIcon(ERRGearKind Kind, int64 Seed, float X, float Y, float S, float Alpha);
	void DrawInventoryWallet(float PX, float PY, float PW, float PH);
	void CloseInventory();
	void EquipGearIndex(int32 Index);

	bool bInventoryOpen = false;
	int32 ActiveTab = 0; // 0 = ITEMS, 1 = WALLET
	int32 InvSel = 0;
	int32 InvScrollRow = 0;
	double InvOpenTime = 0.0;
	void OnMouseScrollUp();
	void OnMouseScrollDown();

	// ---- defeat screen (RRHUD_Defeat.cpp) ----
	void DrawDefeat();
	void UpdateDefeatState();
	bool bDefeatActive = false;
	bool bDefeatRespawned = false;
	double DefeatStart = 0.0;
	double RespawnFadeStart = 0.0;
	int32 PrevMatchState = -1;
	bool bDebugDefeat = false;

	// ---- key bindings / in-match HUD ----
	void BindKeys();
	void OnKeyF2();
	void OnKeyF3();
	void OnKey1();
	void OnKey2();
	void OnKey3();
	void OnKey4();
	void OnKey5();
	void OnKey6();
	void OnKey7();
	void OnKey8();
	void OnKey9();
	void OnKeyEnter();
	void OnKeyC();
	void DrawMatchHUD();
	void DrawCinematicBars();

	// ---- intro video, flash, arena prompt (RRHUD_Intro.cpp) ----
	bool StartIntroVideo();
	void DrawIntroVideo();
	void EndIntroVideo();
	void DrawArenaPrompt();
	void DrawFlash();
public:
	/** Full-screen flash that fades out (teleports, deathblows). */
	void Flash(const FLinearColor& Color, float Duration);
protected:
	double IntroStart = 0.0;
	bool bIntroWasPlaying = false;
	FLinearColor FlashColor = FLinearColor::White;
	double FlashStart = -100.0;
	float FlashDuration = 0.5f;
	UPROPERTY(Transient) TObjectPtr<class UMediaPlayer> IntroPlayer;
	UPROPERTY(Transient) TObjectPtr<class UFileMediaSource> IntroSource;
	UPROPERTY(Transient) TObjectPtr<class UMediaTexture> IntroTexture;
	UPROPERTY(Transient) TObjectPtr<class UMediaSoundComponent> IntroSound;
	void DrawDamageFx();
	void DrawBossHud();

	void OnBossPhaseChanged(class ARRBoss* Boss, int32 Phase);
	void OnBossPerilousWarning(class ARRBoss* Boss, ERRBossAttackKind Kind, float WarningSeconds);

	FDelegateHandle BossPhaseHandle;
	FDelegateHandle BossPerilousHandle;
	float BossPhaseTextEnd = -1.0f;
	float BossPhaseShatterEnd = -1.0f;
	float BossPerilousEnd = -1.0f;
	int32 BossShatteredSegment = -1;
	float BossGhostHealth[10] = {0.0f};

	float DispStamina = 0.0f;
	float DispPoise = 0.0f;
	float GhostStamina = 0.0f;
	float GhostPoise = 0.0f;
	
	float HitPulse = 0.0f;
	float LastPollHealth = -1.0f;
	float DbgLowHealthFraction = -1.0f;

	struct FBloodSplat
	{
		FVector2D Pos;
		float Size;
		float Rotation;
		float Life;
		float MaxLife;
	};
	TArray<FBloodSplat> BloodSplats;
	float LastFlashTime = -1.0f;

	// ---- layout / assets ----
	float UIScale = 1.0f;
	double Now = 0.0;
	float CanvasW = 1920.0f, CanvasH = 1080.0f;

	UPROPERTY(Transient) TObjectPtr<UTexture2D> MenuBackground;
	UPROPERTY(Transient) TArray<TObjectPtr<UTexture2D>> LoadingImages;
	UPROPERTY(Transient) TObjectPtr<UTexture2D> LogoTexture;
	UPROPERTY(Transient) TObjectPtr<UFont> DistanceFont;
	bool bAssetsLoaded = false;
};
