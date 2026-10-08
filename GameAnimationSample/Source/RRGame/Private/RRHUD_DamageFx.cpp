// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRHUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AAAAttributeSet.h"

void ARRHUD::DebugInjectHit(float Fraction)
{
	float Lost = Fraction;
	HitPulse = FMath::Max(HitPulse, FMath::Clamp(Lost * 3.2f, 0.25f, 1.0f));
	if (Lost > 0.25f)
	{
		LastFlashTime = Now;
	}

	if (BloodSplats.Num() < 10)
	{
		FBloodSplat Splat;
		// Random position near edge
		if (FMath::RandBool())
		{
			Splat.Pos.X = FMath::RandRange(0.0f, CanvasW);
			Splat.Pos.Y = FMath::RandBool() ? FMath::RandRange(0.0f, CanvasH * 0.2f) : FMath::RandRange(CanvasH * 0.8f, CanvasH);
		}
		else
		{
			Splat.Pos.X = FMath::RandBool() ? FMath::RandRange(0.0f, CanvasW * 0.2f) : FMath::RandRange(CanvasW * 0.8f, CanvasW);
			Splat.Pos.Y = FMath::RandRange(0.0f, CanvasH);
		}
		Splat.Size = FMath::RandRange(40.0f, 150.0f);
		Splat.Rotation = FMath::RandRange(0.0f, 360.0f);
		Splat.MaxLife = 1.6f;
		Splat.Life = 1.6f;
		BloodSplats.Add(Splat);
	}
}

void ARRHUD::DrawDamageFx()
{
	if (!Canvas) return;

	float HealthFraction = 1.0f;
	float CurHealth = -1.0f;
	float MaxHealth = 1.0f;

	APlayerController* PC = GetOwningPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (Pawn)
	{
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn)) ASC = ASI->GetAbilitySystemComponent();
		else if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState())) ASC = PS_ASI->GetAbilitySystemComponent();

		if (ASC)
		{
			CurHealth = ASC->GetNumericAttribute(UAAAAttributeSet::GetHealthAttribute());
			MaxHealth = ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxHealthAttribute());
			if (MaxHealth > 0.0f)
			{
				HealthFraction = FMath::Clamp(CurHealth / MaxHealth, 0.0f, 1.0f);
			}
		}
	}

	if (DbgLowHealthFraction >= 0.0f)
	{
		HealthFraction = DbgLowHealthFraction;
	}

	if (CurHealth >= 0.0f && LastPollHealth >= 0.0f)
	{
		float Lost = LastPollHealth - CurHealth;
		if (Lost > 0.0f)
		{
			float LostFraction = MaxHealth > 0.0f ? Lost / MaxHealth : 0.0f;
			HitPulse = FMath::Max(HitPulse, FMath::Clamp(LostFraction * 3.2f, 0.25f, 1.0f));
			
			if (LostFraction > 0.25f)
			{
				LastFlashTime = Now;
			}

			if (BloodSplats.Num() < 10)
			{
				FBloodSplat Splat;
				if (FMath::RandBool())
				{
					Splat.Pos.X = FMath::RandRange(0.0f, CanvasW);
					Splat.Pos.Y = FMath::RandBool() ? FMath::RandRange(0.0f, CanvasH * 0.2f) : FMath::RandRange(CanvasH * 0.8f, CanvasH);
				}
				else
				{
					Splat.Pos.X = FMath::RandBool() ? FMath::RandRange(0.0f, CanvasW * 0.2f) : FMath::RandRange(CanvasW * 0.8f, CanvasW);
					Splat.Pos.Y = FMath::RandRange(0.0f, CanvasH);
				}
				Splat.Size = FMath::RandRange(40.0f, 150.0f);
				Splat.Rotation = FMath::RandRange(0.0f, 360.0f);
				Splat.MaxLife = 1.6f;
				Splat.Life = 1.6f;
				BloodSplats.Add(Splat);
			}
		}
	}
	
	if (CurHealth >= 0.0f)
	{
		LastPollHealth = CurHealth;
	}

	float DT = GetWorld()->GetDeltaSeconds();
	HitPulse = FMath::Max(0.0f, HitPulse * FMath::Exp(-2.0f * DT)); // half-life approx 0.35s

	for (int32 i = BloodSplats.Num() - 1; i >= 0; --i)
	{
		BloodSplats[i].Life -= DT;
		if (BloodSplats[i].Life <= 0.0f)
		{
			BloodSplats.RemoveAtSwap(i);
		}
	}

	float LowHealthTerm = FMath::SmoothStep(0.55f, 0.1f, HealthFraction);
	
	// Heartbeat pulse: two quick beats per second scaled by 1 - healthFraction
	float BeatTime = FMath::Fmod(Now, 1.0);
	float HeartPulse = 0.0f;
	if (BeatTime < 0.2f) HeartPulse = FMath::Sin(BeatTime * PI / 0.2f);
	else if (BeatTime > 0.3f && BeatTime < 0.5f) HeartPulse = FMath::Sin((BeatTime - 0.3f) * PI / 0.2f);
	
	float PersistentPulse = LowHealthTerm * HeartPulse * (1.0f - HealthFraction);
	float EffectAlpha = (0.25f + 0.55f * HitPulse) + PersistentPulse;
	if (EffectAlpha > 1.0f) EffectAlpha = 1.0f;

	FLinearColor BloodColor(0.55f, 0.02f, 0.02f, 0.0f);

	if (HealthFraction < 0.25f)
	{
		float DesatAlpha = (0.25f - HealthFraction) / 0.25f * 0.28f;
		Box(0, 0, CanvasW, CanvasH, FLinearColor(0.0f, 0.0f, 0.0f, DesatAlpha));
	}

	if (EffectAlpha > 0.01f)
	{
		float EdgeThickness = HealthFraction < 0.25f ? CanvasW * 0.15f : CanvasW * 0.08f;
		FLinearColor InnerCol(BloodColor.R, BloodColor.G, BloodColor.B, 0.0f);
		FLinearColor OuterCol(BloodColor.R, BloodColor.G, BloodColor.B, EffectAlpha);

		VGrad(0, 0, CanvasW, EdgeThickness, OuterCol, InnerCol);
		VGrad(0, CanvasH - EdgeThickness, CanvasW, EdgeThickness, InnerCol, OuterCol);
		HGrad(0, 0, EdgeThickness, CanvasH, OuterCol, InnerCol);
		HGrad(CanvasW - EdgeThickness, 0, EdgeThickness, CanvasH, InnerCol, OuterCol);

		auto DrawVeins = [&](float EdgeX, float EdgeY, bool bVertical)
		{
			for (int32 i = 0; i < 5; ++i)
			{
				int32 Hash = (int32)EdgeX * 13 + (int32)EdgeY * 7 + i * 31;
				float PosOffset = (Hash % 100) / 100.0f;
				float Length = 50.0f + (Hash % 150);
				float Angle = (Hash % 60) - 30.0f;
				
				float X = bVertical ? EdgeX : EdgeX + PosOffset * CanvasW;
				float Y = bVertical ? EdgeY + PosOffset * CanvasH : EdgeY;
				
				if (bVertical && EdgeX > 0) Angle += 180.0f;
				if (!bVertical && EdgeY > 0) Angle += 90.0f;
				else if (!bVertical) Angle -= 90.0f;

				RotRect(X, Y, U(4.0f), U(Length), Angle, OuterCol);
				RotRect(X + U(FMath::Sin(Angle)*Length*0.5f), Y - U(FMath::Cos(Angle)*Length*0.5f), U(2.0f), U(Length*0.8f), Angle + 15.0f, OuterCol);
			}
		};

		DrawVeins(0.0f, 0.0f, true);
		DrawVeins(CanvasW, 0.0f, true);
		DrawVeins(0.0f, 0.0f, false);
		DrawVeins(0.0f, CanvasH, false);
	}

	for (const FBloodSplat& Splat : BloodSplats)
	{
		float SplatAlpha = Splat.Life / Splat.MaxLife;
		FLinearColor SplatCol(BloodColor.R, BloodColor.G, BloodColor.B, SplatAlpha * 0.8f);
		int32 Seed = (int32)Splat.Pos.X * 17 + (int32)Splat.Pos.Y * 23;
		
		for (int32 i = 0; i < 6; ++i)
		{
			float OffsetX = U((Seed % 40) - 20.0f);
			float OffsetY = U(((Seed >> 2) % 40) - 20.0f);
			float S = U(Splat.Size * (0.5f + (Seed % 50) / 100.0f));
			float A = Splat.Rotation + (Seed % 90);
			RotRect(Splat.Pos.X + OffsetX, Splat.Pos.Y + OffsetY, S, S * 0.6f, A, SplatCol);
			Seed = (Seed * 13) + 7;
		}
	}

	if (LastFlashTime > 0.0f && (Now - LastFlashTime) < 0.06f)
	{
		Box(0, 0, CanvasW, CanvasH, FLinearColor(1.0f, 1.0f, 1.0f, 0.04f));
	}
}
