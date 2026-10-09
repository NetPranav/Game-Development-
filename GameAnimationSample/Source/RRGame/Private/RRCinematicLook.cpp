// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRCinematicLook.h"
#include "Components/PostProcessComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<int32> CVarRRLook(TEXT("rr.look"), 1, TEXT("1 = cinematic lens post process on, 0 = off"));
static TAutoConsoleVariable<float> CVarRRLookFstop(TEXT("rr.look.fstop"), 4.0f, TEXT("Aperture of the gameplay depth of field (lower = blurrier background)"));

ARRCinematicLook::ARRCinematicLook()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	RootComponent = PostProcess;
	PostProcess->bUnbound = true;
	PostProcess->Priority = 5.0f;   // above a typical level volume (0) for the lens values only

	FPostProcessSettings& S = PostProcess->Settings;
	// lens
	S.bOverride_BloomIntensity = true;              S.BloomIntensity = 0.85f;
	S.bOverride_BloomThreshold = true;              S.BloomThreshold = -1.0f;
	S.bOverride_VignetteIntensity = true;           S.VignetteIntensity = 0.45f;
	S.bOverride_SceneFringeIntensity = true;        S.SceneFringeIntensity = 0.35f;   // chromatic aberration, edges only
	S.bOverride_FilmGrainIntensity = true;          S.FilmGrainIntensity = 0.12f;
	S.bOverride_MotionBlurAmount = true;            S.MotionBlurAmount = 0.3f;
	S.bOverride_MotionBlurMax = true;               S.MotionBlurMax = 3.0f;
	// a touch of contrast, colour left to the level
	S.bOverride_ColorContrast = true;               S.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);
	// gameplay depth of field: focus tracks the player
	S.bOverride_DepthOfFieldFocalDistance = true;   S.DepthOfFieldFocalDistance = FocusDistance;
	S.bOverride_DepthOfFieldFstop = true;           S.DepthOfFieldFstop = 4.0f;
	S.bOverride_DepthOfFieldSensorWidth = true;     S.DepthOfFieldSensorWidth = 24.576f;
	S.bOverride_DepthOfFieldDepthBlurAmount = true; S.DepthOfFieldDepthBlurAmount = 1.0f;
	S.bOverride_DepthOfFieldDepthBlurRadius = true; S.DepthOfFieldDepthBlurRadius = 0.0f;
}

void ARRCinematicLook::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PostProcess->bEnabled = CVarRRLook.GetValueOnGameThread() != 0;
	if (!PostProcess->bEnabled) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (PC && PC->PlayerCameraManager && Pawn)
	{
		const FVector Cam = PC->PlayerCameraManager->GetCameraLocation();
		// focus slightly past the player so the opponent in front stays sharp too
		const float Target = FVector::Dist(Cam, Pawn->GetActorLocation() + FVector(0, 0, 60.0f)) + 120.0f;
		const float Real = FMath::Min(FApp::GetDeltaTime(), 0.1);
		FocusDistance = FMath::FInterpTo(FocusDistance, Target, Real, 6.0f);
	}
	PostProcess->Settings.DepthOfFieldFocalDistance = FocusDistance;
	PostProcess->Settings.DepthOfFieldFstop = FMath::Max(0.7f, CVarRRLookFstop.GetValueOnGameThread());
}
