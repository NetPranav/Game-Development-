// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRHUD.h"
#include "RRBoss.h"
#include "RREnemyDirector.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ARRHUD::OnBossPhaseChanged(ARRBoss* Boss, int32 Phase)
{
	BossPhaseTextEnd = Now + 1.5f;
	BossPhaseShatterEnd = Now + 0.6f;
	BossShatteredSegment = Boss->GetSegmentsRemaining();
}

void ARRHUD::OnBossPerilousWarning(ARRBoss* Boss, ERRBossAttackKind Kind, float WarningSeconds)
{
	BossPerilousEnd = Now + WarningSeconds;
}

void ARRHUD::DrawBossHud()
{
	if (!Canvas || !GEngine || bDefeatActive || Screen != ERRScreen::Playing) return;

	URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>();
	if (!Director) return;

	ARRBoss* Boss = nullptr;
	for (ARREnemy* Enemy : Director->GetLivingEnemies())
	{
		Boss = Cast<ARRBoss>(Enemy);
		if (Boss) break;
	}
	
	if (!Boss) return;

	float DT = GetWorld()->GetDeltaSeconds();
	int32 TotalSegs = Boss->HealthSegments;
	int32 RemSegs = Boss->GetSegmentsRemaining();
	float HealthFrac = Boss->GetSegmentFraction();

	float HUDWidth = CanvasW * 0.52f;
	float CX = CanvasW * 0.5f;
	float CY = CanvasH - 120.0f;

	// Boss Name
	FString NameStr = Boss->GetBossName().ToString().ToUpper();
	float NW = TxtWidth(NameStr, 1.0f, 2.0f);
	Txt(NameStr, CX - NW * 0.5f, CY - 30.0f, 1.0f, FLinearColor::White, 0.0f, 2.0f);

	// Health Segments
	float SegSpacing = 10.0f;
	float TotalSegWidth = HUDWidth;
	float SegW = (TotalSegWidth - FMath::Max(0, TotalSegs - 1) * SegSpacing) / FMath::Max(1, TotalSegs);
	float SegH = 10.0f;

	float StartX = CX - HUDWidth * 0.5f;

	for (int32 i = 0; i < TotalSegs; ++i)
	{
		float SX = StartX + i * (SegW + SegSpacing);
		float SY = CY;

		float TargetFrac = 0.0f;
		if (i < RemSegs - 1) TargetFrac = 1.0f;
		else if (i == RemSegs - 1) TargetFrac = HealthFrac;
		else TargetFrac = 0.0f;
		
		BossGhostHealth[i] = FMath::Lerp(BossGhostHealth[i], TargetFrac, 3.0f * DT);
		if (BossGhostHealth[i] < 1.0f && TargetFrac >= 1.0f) BossGhostHealth[i] = TargetFrac;
		if (BossGhostHealth[i] < TargetFrac) BossGhostHealth[i] = TargetFrac;

		// Empty background
		Box(SX, SY, SegW, SegH, FLinearColor(0.05f, 0.05f, 0.05f, 0.8f));
		Frame(SX, SY, SegW, SegH, 1.0f, FLinearColor(0.2f, 0.2f, 0.2f, 0.9f));

		if (i == BossShatteredSegment && Now < BossPhaseShatterEnd)
		{
			// Shatter animation
			float T = 1.0f - (BossPhaseShatterEnd - Now) / 0.6f;
			float ShatterW = SegW / 6.0f;
			for (int32 j = 0; j < 6; ++j)
			{
				float FallY = SY + T * (100.0f + j * 20.0f);
				float FallX = SX + j * ShatterW + (j % 2 == 0 ? -T * 30.0f : T * 30.0f);
				Box(FallX, FallY, ShatterW - 2.0f, SegH, FLinearColor(1.0f, 1.0f, 1.0f, 1.0f - T));
			}
		}
		else if (TargetFrac > 0.0f || BossGhostHealth[i] > 0.0f)
		{
			// Ghost white
			Box(SX, SY, SegW * BossGhostHealth[i], SegH, FLinearColor(1.0f, 1.0f, 1.0f, 0.8f));
			// Actual red health
			Box(SX, SY, SegW * TargetFrac, SegH, FLinearColor(0.7f, 0.0f, 0.0f, 1.0f));
		}
	}

	// Posture Bar
	float PostureFrac = Boss->GetPostureFraction();
	float PostureW = HUDWidth * 0.8f;
	float PostureH = 4.0f;
	float PostureCX = CX;
	float PostureCY = CY + 20.0f;

	Box(PostureCX - PostureW * 0.5f, PostureCY, PostureW, PostureH, FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
	
	FLinearColor PColor = FMath::Lerp(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), PostureFrac);
	bool bBroken = Boss->IsPostureBroken();
	
	if (bBroken)
	{
		if (FMath::Fmod(Now, 0.2) > 0.1) PColor = FLinearColor::White;
		Txt(TEXT("BREAK"), PostureCX - TxtWidth(TEXT("BREAK"), 1.0f) * 0.5f, PostureCY - 15.0f, 1.0f, FLinearColor::Red, 0.0f, 1.0f);
	}
	
	float FillW = PostureW * PostureFrac;
	Box(PostureCX - FillW * 0.5f, PostureCY, FillW, PostureH, PColor);

	// Deathblow prompt
	if (Boss->IsDeathblowReady() || bBroken)
	{
		float Pulse = 0.5f + 0.5f * FMath::Sin(Now * 10.0f);
		FLinearColor DBColor = FLinearColor::Red;
		DBColor.A = Pulse;
		FString DBStr = TEXT("DEATHBLOW");
		Txt(DBStr, CX - TxtWidth(DBStr, 2.0f) * 0.5f, CanvasH * 0.6f, 2.0f, DBColor);
	}

	// Perilous glyph
	if (Now < BossPerilousEnd)
	{
		FVector HeadLoc = Boss->GetActorLocation() + FVector(0,0,140.f);
		FVector ScreenLoc = Project(HeadLoc);
		if (ScreenLoc.Z > 0.0f)
		{
			float Pulse = 0.7f + 0.3f * FMath::Sin(Now * 20.0f);
			FLinearColor PCol = FLinearColor::Red;
			PCol.A = Pulse;
			
			float GX = ScreenLoc.X;
			float GY = ScreenLoc.Y - 50.0f;
			float GS = 40.0f;
			
			RotRect(GX, GY, GS, GS, 45.0f, PCol);
			Line(GX - GS*0.6f, GY - GS*0.6f, GX + GS*0.6f, GY + GS*0.6f, 3.0f, PCol);
			Line(GX - GS*0.6f, GY + GS*0.6f, GX + GS*0.6f, GY - GS*0.6f, 3.0f, PCol);
			Line(GX + GS*0.8f, GY - GS*0.2f, GX + GS*1.2f, GY - GS*0.5f, 2.0f, PCol);
			Line(GX + GS*0.8f, GY + GS*0.2f, GX + GS*1.2f, GY + GS*0.5f, 2.0f, PCol);
		}
		
		Box(0, 0, CanvasW, 10.0f, FLinearColor::Red);
		Box(0, CanvasH - 10.0f, CanvasW, 10.0f, FLinearColor::Red);
		Box(0, 0, 10.0f, CanvasH, FLinearColor::Red);
		Box(CanvasW - 10.0f, 0, 10.0f, CanvasH, FLinearColor::Red);
	}

	// Phase change
	if (Now < BossPhaseTextEnd)
	{
		float Alpha = FMath::Clamp((BossPhaseTextEnd - Now) / 0.5f, 0.0f, 1.0f);
		if ((BossPhaseTextEnd - Now) > 1.0f) Alpha = FMath::Clamp(1.0f - ((BossPhaseTextEnd - Now) - 1.0f) / 0.5f, 0.0f, 1.0f);
		
		FString PhaseStr = TEXT("PHASE II");
		FLinearColor PhColor = FLinearColor::White;
		PhColor.A = Alpha;
		Txt(PhaseStr, CX - TxtWidth(PhaseStr, 2.0f, 4.0f) * 0.5f, CanvasH * 0.4f, 2.0f, PhColor, 0.0f, 4.0f);
	}
}
