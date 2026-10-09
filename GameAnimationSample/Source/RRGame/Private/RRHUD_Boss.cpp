// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Boss presentation, souls-like: name + health bar across the top, remaining "lives" as orbs, posture bar under it,
// trailing damage, a drain/refill when the boss falls and rises again, perilous glyph and deathblow mark over its head.

#include "RRHUD.h"
#include "RRBoss.h"
#include "RRGameMode.h"
#include "RREnemyDirector.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ARRHUD::OnBossPhaseChanged(ARRBoss* Boss, int32 Phase)
{
	BossPhaseTextEnd = Now + 3.2f;
	BossPhaseShatterEnd = Now + 0.9f;
	BossShatteredSegment = Boss ? Boss->GetSegmentsRemaining() : -1;
}

void ARRHUD::OnBossPerilousWarning(ARRBoss* Boss, ERRBossAttackKind Kind, float WarningSeconds)
{
	BossPerilousEnd = Now + FMath::Max(0.4f, WarningSeconds);
}

void ARRHUD::DrawBossHud()
{
	if (!Canvas || bDefeatActive || Screen != ERRScreen::Playing) return;
	ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
	ARRBoss* Boss = GM ? GM->FindBoss() : nullptr;
	if (!Boss || !GM) return;

	// only while it matters: the duel (and its intro taunt). Fades in.
	static float Show = 0.0f;
	const bool bWant = GM->GetMatchState() == ERRMatchState::InProgress || (GM->GetArena() && !GM->CanEnterArena() && GM->GetMatchState() == ERRMatchState::PreMatch && !GM->IsFrontEndPending());
	const float DT = FMath::Min((float)FApp::GetDeltaTime(), 0.1f);
	Show = FMath::FInterpConstantTo(Show, bWant ? 1.0f : 0.0f, DT, 2.5f);
	if (Show <= 0.01f) return;
	const float A = Show;

	const float CX = CanvasW * 0.5f;
	const float BarW = U(860.0f);
	const float BarH = FMath::Max(6.0f, U(9.0f));
	const float BarX = CX - BarW * 0.5f;
	const float BarY = U(74.0f);

	// soft dark band behind the top HUD
	VGrad(0.0f, 0.0f, CanvasW, U(150.0f), FLinearColor(0, 0, 0, 0.55f * A), FLinearColor(0, 0, 0, 0.0f), 24);

	// name
	FString Name = Boss->GetBossName().ToString().ToUpper();
	Txt(Name, CX, U(26.0f), 26.0f, FLinearColor(0.93f, 0.9f, 0.84f, A), 0.5f, 9.0f);

	// lives: one orb per remaining segment (Sekiro), left of the bar
	const int32 Total = FMath::Max(1, Boss->HealthSegments);
	const int32 Rem = FMath::Clamp(Boss->GetSegmentsRemaining(), 0, Total);
	for (int32 i = 0; i < Total; ++i)
	{
		const float OX = BarX - U(26.0f) - i * U(24.0f);
		const float OY = BarY + BarH * 0.5f;
		const bool bLive = i < Rem;
		const bool bShatter = (i == Rem) && Now < BossPhaseShatterEnd;
		RotRect(OX, OY, U(13.0f), U(13.0f), 45.0f, FLinearColor(0.1f, 0.02f, 0.02f, 0.9f * A));
		if (bLive) RotRect(OX, OY, U(9.0f), U(9.0f), 45.0f, FLinearColor(0.85f, 0.08f, 0.06f, A));
		if (bShatter)
		{
			const float T = 1.0f - (float)((BossPhaseShatterEnd - Now) / 0.9f);
			for (int32 k = 0; k < 6; ++k)
			{
				const float Ang = k * PI / 3.0f;
				RotRect(OX + FMath::Cos(Ang) * T * U(30.0f), OY + FMath::Sin(Ang) * T * U(30.0f), U(5.0f), U(5.0f), 45.0f + T * 180.0f, FLinearColor(1.0f, 0.4f, 0.3f, (1.0f - T) * A));
			}
		}
	}

	// health: frame, trailing white ghost, red fill
	const float Frac = Rem > 0 ? FMath::Clamp(Boss->GetSegmentFraction(), 0.0f, 1.0f) : 0.0f;
	float& Ghost = BossGhostHealth[0];
	float& Disp = BossGhostHealth[1];
	if (Frac > Disp + 0.02f) { Disp = FMath::FInterpConstantTo(Disp, Frac, DT, 0.55f); Ghost = Disp; }   // refill (rising) animates
	else { Disp = Frac; }
	Ghost = Ghost < Disp ? Disp : FMath::FInterpTo(Ghost, Disp, DT, Now - BossPerilousEnd > 0.0 ? 2.2f : 2.2f);

	Box(BarX - U(2.0f), BarY - U(2.0f), BarW + U(4.0f), BarH + U(4.0f), FLinearColor(0, 0, 0, 0.75f * A));
	Box(BarX, BarY, BarW, BarH, FLinearColor(0.08f, 0.03f, 0.03f, 0.9f * A));
	Box(BarX, BarY, BarW * Ghost, BarH, FLinearColor(0.95f, 0.88f, 0.75f, 0.8f * A));
	HGrad(BarX, BarY, BarW * Disp, BarH, FLinearColor(0.5f, 0.03f, 0.03f, A), FLinearColor(0.85f, 0.1f, 0.06f, A), 16);
	Box(BarX, BarY, BarW * Disp, FMath::Max(1.0f, BarH * 0.25f), FLinearColor(1.0f, 0.55f, 0.45f, 0.35f * A));   // top sheen
	Frame(BarX - U(3.0f), BarY - U(3.0f), BarW + U(6.0f), BarH + U(6.0f), 1.0f, FLinearColor(0.75f, 0.62f, 0.38f, 0.55f * A));

	// posture: grows from the centre, orange -> red, flashes white when broken
	const float Post = Boss->GetPostureFraction();
	const bool bBroken = Boss->IsPostureBroken() || Boss->IsDeathblowReady();
	const float PW = BarW * 0.62f;
	const float PH = FMath::Max(4.0f, U(6.0f));
	const float PY = BarY + BarH + U(14.0f);
	Box(CX - PW * 0.5f, PY, PW, PH, FLinearColor(0, 0, 0, 0.55f * A));
	FLinearColor PC = FMath::Lerp(FLinearColor(1.0f, 0.62f, 0.12f, A), FLinearColor(1.0f, 0.12f, 0.05f, A), Post);
	if (bBroken && FMath::Fmod(Now, 0.24) < 0.12) PC = FLinearColor(1, 1, 1, A);
	const float Fill = bBroken ? PW : PW * Post;
	Box(CX - Fill * 0.5f, PY, Fill, PH, PC);
	Box(CX - U(1.0f), PY - U(3.0f), U(2.0f), PH + U(6.0f), FLinearColor(1, 1, 1, 0.5f * A));   // centre tick

	// over-the-head marks
	const FVector Head = Boss->GetActorLocation() + FVector(0.0f, 0.0f, 165.0f);
	const FVector S = Project(Head);
	if (S.Z > 0.0f)
	{
		if (bBroken)
		{
			// deathblow mark: red diamond with white core
			const float Pulse = 0.75f + 0.25f * FMath::Sin((float)Now * 9.0f);
			RotRect(S.X, S.Y, U(26.0f) * Pulse, U(26.0f) * Pulse, 45.0f, FLinearColor(0.9f, 0.05f, 0.05f, 0.9f));
			RotRect(S.X, S.Y, U(10.0f), U(10.0f), 45.0f, FLinearColor(1, 1, 1, 0.95f));
		}
		else if (Now < BossPerilousEnd)
		{
			// perilous warning: red glyph
			const float Pulse = 0.7f + 0.3f * FMath::Sin((float)Now * 18.0f);
			const FLinearColor RC(1.0f, 0.08f, 0.05f, Pulse);
			const float G = U(30.0f);
			RotRect(S.X, S.Y - G * 0.4f, G, G, 45.0f, FLinearColor(0.2f, 0.0f, 0.0f, 0.6f));
			Line(S.X - G * 0.45f, S.Y - G * 0.85f, S.X + G * 0.45f, S.Y + G * 0.05f, FMath::Max(2.0f, U(4.0f)), RC);
			Line(S.X + G * 0.45f, S.Y - G * 0.85f, S.X - G * 0.45f, S.Y + G * 0.05f, FMath::Max(2.0f, U(4.0f)), RC);
		}
	}

	// perilous: thin red pulse on the screen edges
	if (Now < BossPerilousEnd)
	{
		const float E = 0.35f + 0.25f * FMath::Sin((float)Now * 18.0f);
		HGrad(0, 0, U(90.0f), CanvasH, FLinearColor(0.8f, 0.0f, 0.0f, E), FLinearColor(0.8f, 0.0f, 0.0f, 0.0f), 12);
		HGrad(CanvasW - U(90.0f), 0, U(90.0f), CanvasH, FLinearColor(0.8f, 0.0f, 0.0f, 0.0f), FLinearColor(0.8f, 0.0f, 0.0f, E), 12);
	}

	if (bBroken)
	{
		const float Pulse = 0.55f + 0.45f * FMath::Sin((float)Now * 8.0f);
		Txt(TEXT("DEATHBLOW"), CX, CanvasH * 0.78f, 24.0f, FLinearColor(0.95f, 0.15f, 0.1f, Pulse), 0.5f, 14.0f);
	}

	// the boss rose again
	if (Now < BossPhaseTextEnd)
	{
		const float T = (float)(BossPhaseTextEnd - Now);
		const float FA = FMath::Clamp(FMath::Min(T / 0.6f, (3.2f - T) / 0.6f), 0.0f, 1.0f);
		Txt(TEXT("IT RISES AGAIN"), CX, CanvasH * 0.32f, 34.0f, FLinearColor(0.95f, 0.88f, 0.8f, FA), 0.5f, 16.0f);
		Box(CX - U(160.0f) * FA, CanvasH * 0.32f + U(56.0f), U(320.0f) * FA, FMath::Max(1.0f, U(2.0f)), FLinearColor(0.85f, 0.12f, 0.1f, FA));
	}
}
