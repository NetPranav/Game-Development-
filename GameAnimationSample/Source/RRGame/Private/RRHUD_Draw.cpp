// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Shared canvas helpers + input polling for ARRHUD. Everything is laid out in a 1920x1080 reference space (U()).

#include "RRHUD.h"
#include "RRGame.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "RenderResource.h"

void ARRHUD::LoadUIAssets()
{
	if (bAssetsLoaded) return;
	bAssetsLoaded = true;

	MenuBackground = LoadObject<UTexture2D>(nullptr, TEXT("/Game/RR/UI/T_RR_Start.T_RR_Start"));
	LogoTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/RR/UI/T_KaizeenLogo.T_KaizeenLogo"));
	for (int32 i = 1; i <= 4; ++i)
	{
		const FString Path = FString::Printf(TEXT("/Game/RR/UI/T_RR_Load_%d.T_RR_Load_%d"), i, i);
		if (UTexture2D* T = LoadObject<UTexture2D>(nullptr, *Path))
		{
			LoadingImages.Add(T);
		}
	}
	DistanceFont = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
	UE_LOG(LogRR, Log, TEXT("UI assets: menu bg %s, %d loading images, logo %s, distance font %s"),
		MenuBackground ? TEXT("ok") : TEXT("MISSING"), LoadingImages.Num(), LogoTexture ? TEXT("custom") : TEXT("procedural"), DistanceFont ? TEXT("ok") : TEXT("MISSING"));
}

void ARRHUD::BeginFrame()
{
	Now = FPlatformTime::Seconds();
	CanvasW = Canvas ? Canvas->SizeX : 1920.0f;
	CanvasH = Canvas ? Canvas->SizeY : 1080.0f;
	UIScale = FMath::Clamp(FMath::Min(CanvasW / 1920.0f, CanvasH / 1080.0f), 0.4f, 2.5f);
	LoadUIAssets();
	PollInput();
}

UFont* ARRHUD::BodyFont() const
{
	return DistanceFont ? DistanceFont.Get() : GEngine->GetLargeFont();
}

float ARRHUD::TxtWidth(const FString& Text, float Size, float Tracking)
{
	UFont* F = BodyFont();
	if (!F || Text.IsEmpty()) return 0.0f;
	const float Scale = U(Size) / FMath::Max(1.0f, (float)F->GetMaxCharHeight());
	float W = 0.0f, H = 0.0f;
	if (Tracking <= 0.0f)
	{
		GetTextSize(Text, W, H, F, Scale);
		return W;
	}
	for (int32 i = 0; i < Text.Len(); ++i)
	{
		const FString C = Text.Mid(i, 1);
		if (C == TEXT(" "))
		{
			W += U(Size) * 0.32f + U(Tracking);
			continue;
		}
		float CW = 0.0f, CH = 0.0f;
		GetTextSize(C, CW, CH, F, Scale);
		W += CW + U(Tracking);
	}
	return FMath::Max(0.0f, W - U(Tracking));
}

void ARRHUD::Txt(const FString& Text, float X, float Y, float Size, const FLinearColor& Color, float Align, float Tracking, bool bShadow)
{
	UFont* F = BodyFont();
	if (!F || Text.IsEmpty() || Color.A <= 0.003f) return;
	const float Scale = U(Size) / FMath::Max(1.0f, (float)F->GetMaxCharHeight());
	const float W = TxtWidth(Text, Size, Tracking);
	float PX = X - W * Align;
	const float Off = FMath::Max(1.0f, U(Size) * 0.04f);
	FLinearColor Sh(0.0f, 0.0f, 0.0f, Color.A * 0.65f);

	if (Tracking <= 0.0f)
	{
		if (bShadow) DrawText(Text, Sh, PX + Off, Y + Off, F, Scale, false);
		DrawText(Text, Color, PX, Y, F, Scale, false);
		return;
	}
	for (int32 i = 0; i < Text.Len(); ++i)
	{
		const FString C = Text.Mid(i, 1);
		if (C == TEXT(" "))
		{
			PX += U(Size) * 0.32f + U(Tracking);
			continue;
		}
		float CW = 0.0f, CH = 0.0f;
		GetTextSize(C, CW, CH, F, Scale);
		if (bShadow) DrawText(C, Sh, PX + Off, Y + Off, F, Scale, false);
		DrawText(C, Color, PX, Y, F, Scale, false);
		PX += CW + U(Tracking);
	}
}

void ARRHUD::Box(float X, float Y, float W, float H, const FLinearColor& C)
{
	if (C.A <= 0.002f || W <= 0.0f || H <= 0.0f) return;
	DrawRect(C, X, Y, W, H);
}

void ARRHUD::Frame(float X, float Y, float W, float H, float Thick, const FLinearColor& C)
{
	const float T = FMath::Max(1.0f, Thick);
	Box(X, Y, W, T, C);
	Box(X, Y + H - T, W, T, C);
	Box(X, Y, T, H, C);
	Box(X + W - T, Y, T, H, C);
}

void ARRHUD::Brackets(float X, float Y, float W, float H, float Len, float Thick, const FLinearColor& C)
{
	const float T = FMath::Max(1.0f, Thick);
	// top-left
	Box(X, Y, Len, T, C); Box(X, Y, T, Len, C);
	// top-right
	Box(X + W - Len, Y, Len, T, C); Box(X + W - T, Y, T, Len, C);
	// bottom-left
	Box(X, Y + H - T, Len, T, C); Box(X, Y + H - Len, T, Len, C);
	// bottom-right
	Box(X + W - Len, Y + H - T, Len, T, C); Box(X + W - T, Y + H - Len, T, Len, C);
}

void ARRHUD::VGrad(float X, float Y, float W, float H, const FLinearColor& Top, const FLinearColor& Bottom, int32 Steps)
{
	Steps = FMath::Max(1, Steps);
	for (int32 i = 0; i < Steps; ++i)
	{
		const float A = (i + 0.5f) / Steps;
		const float Y0 = FMath::RoundToFloat(Y + H * i / Steps);
		const float Y1 = FMath::RoundToFloat(Y + H * (i + 1) / Steps);
		Box(X, Y0, W, Y1 - Y0, FMath::Lerp(Top, Bottom, A)); // exact edges: overlapping alpha strips show as grid lines
	}
}

void ARRHUD::HGrad(float X, float Y, float W, float H, const FLinearColor& Left, const FLinearColor& Right, int32 Steps)
{
	Steps = FMath::Max(1, Steps);
	for (int32 i = 0; i < Steps; ++i)
	{
		const float A = (i + 0.5f) / Steps;
		const float X0 = FMath::RoundToFloat(X + W * i / Steps);
		const float X1 = FMath::RoundToFloat(X + W * (i + 1) / Steps);
		Box(X0, Y, X1 - X0, H, FMath::Lerp(Left, Right, A));
	}
}

void ARRHUD::Vignette(float Strength)
{
	const FLinearColor Dark(0.0f, 0.0f, 0.0f, Strength);
	const FLinearColor Clear(0.0f, 0.0f, 0.0f, 0.0f);
	const float EdgeX = CanvasW * 0.30f;
	const float EdgeY = CanvasH * 0.34f;
	HGrad(0.0f, 0.0f, EdgeX, CanvasH, Dark, Clear, 20);
	HGrad(CanvasW - EdgeX, 0.0f, EdgeX, CanvasH, Clear, Dark, 20);
	VGrad(0.0f, 0.0f, CanvasW, EdgeY, Dark, Clear, 20);
	VGrad(0.0f, CanvasH - EdgeY, CanvasW, EdgeY, Clear, Dark, 20);
}

void ARRHUD::Cover(UTexture2D* Tex, float Alpha, float Zoom, float PanX, float PanY, float X, float Y, float W, float H)
{
	if (!Tex || Alpha <= 0.003f) return;
	const float TW = FMath::Max(1, Tex->GetSizeX());
	const float TH = FMath::Max(1, Tex->GetSizeY());
	const float TA = TW / TH;
	const float DA = W / FMath::Max(1.0f, H);
	float UW = 1.0f, VH = 1.0f;
	if (DA > TA) VH = TA / DA; else UW = DA / TA;
	Zoom = FMath::Max(1.0f, Zoom);
	UW /= Zoom;
	VH /= Zoom;
	const float U0 = (1.0f - UW) * (0.5f + 0.5f * FMath::Clamp(PanX, -1.0f, 1.0f));
	const float V0 = (1.0f - VH) * (0.5f + 0.5f * FMath::Clamp(PanY, -1.0f, 1.0f));
	DrawTexture(Tex, X, Y, W, H, U0, V0, UW, VH, FLinearColor(1.0f, 1.0f, 1.0f, Alpha), EBlendMode::BLEND_Translucent);
}

void ARRHUD::RotRect(float CX, float CY, float W, float H, float AngleDeg, const FLinearColor& Col)
{
	if (!Canvas || Col.A <= 0.002f) return;
	FCanvasTileItem Tile(FVector2D(CX - W * 0.5f, CY - H * 0.5f), GWhiteTexture, FVector2D(W, H), Col);
	Tile.Rotation = FRotator(0.0f, AngleDeg, 0.0f);
	Tile.PivotPoint = FVector2D(0.5f, 0.5f);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);
}

void ARRHUD::Line(float X0, float Y0, float X1, float Y1, float Thick, const FLinearColor& C)
{
	const float DX = X1 - X0, DY = Y1 - Y0;
	const float Len = FMath::Sqrt(DX * DX + DY * DY);
	if (Len < 0.5f) return;
	RotRect((X0 + X1) * 0.5f, (Y0 + Y1) * 0.5f, Len, FMath::Max(1.0f, Thick), FMath::RadiansToDegrees(FMath::Atan2(DY, DX)), C);
}

void ARRHUD::Tri(FVector2D A, FVector2D B, FVector2D C, const FLinearColor& Col)
{
	if (!Canvas || Col.A <= 0.002f) return;
	FCanvasTriangleItem T(A, B, C, GWhiteTexture);
	T.SetColor(Col);
	T.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(T);
}

void ARRHUD::Embers(float Alpha)
{
	const int32 N = 46;
	for (int32 i = 0; i < N; ++i)
	{
		const float R1 = FMath::Frac(FMath::Sin(i * 12.9898f) * 43758.5453f);
		const float R2 = FMath::Frac(FMath::Sin(i * 78.233f) * 12345.678f);
		const float R3 = FMath::Frac(FMath::Sin(i * 39.425f) * 98765.4321f);
		const float Speed = 18.0f + R2 * 40.0f;
		const float Period = CanvasH * 1.15f;
		const float Yoff = FMath::Fmod((float)Now * Speed + R3 * Period, Period);
		const float Y = CanvasH - Yoff;
		const float X = R1 * CanvasW + FMath::Sin((float)Now * (0.4f + R2) + i) * U(22.0f);
		const float Tw = 0.45f + 0.55f * FMath::Abs(FMath::Sin((float)Now * (1.2f + R3 * 2.0f) + i));
		const float S = U(1.5f + R3 * 2.6f);
		Box(X, Y, S, S, FLinearColor(1.0f, 0.55f + R2 * 0.25f, 0.18f, Alpha * Tw * (0.25f + R1 * 0.5f)));
	}
}

void ARRHUD::DrawLogoMark(float CX, float CY, float Size, float Alpha)
{
	if (LogoTexture)
	{
		DrawTexture(LogoTexture, CX - Size * 0.5f, CY - Size * 0.5f, Size, Size, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor(1, 1, 1, Alpha), EBlendMode::BLEND_Translucent);
		return;
	}
	const FLinearColor Steel(0.86f, 0.89f, 0.92f, Alpha);
	const FLinearColor Dim(0.86f, 0.89f, 0.92f, Alpha * 0.35f);
	const FLinearColor Amber(1.0f, 0.62f, 0.16f, Alpha);
	const float R = Size * 0.5f;
	const float T = U(2.5f);
	// outer + inner diamond
	const FVector2D P[4] = { {CX, CY - R}, {CX + R, CY}, {CX, CY + R}, {CX - R, CY} };
	for (int32 i = 0; i < 4; ++i) Line(P[i].X, P[i].Y, P[(i + 1) % 4].X, P[(i + 1) % 4].Y, T, Steel);
	const float R2 = R * 0.84f;
	const FVector2D Q[4] = { {CX, CY - R2}, {CX + R2, CY}, {CX, CY + R2}, {CX - R2, CY} };
	for (int32 i = 0; i < 4; ++i) Line(Q[i].X, Q[i].Y, Q[(i + 1) % 4].X, Q[(i + 1) % 4].Y, T * 0.5f, Dim);
	// stylised K
	const float K = Size;
	Line(CX - 0.20f * K, CY - 0.34f * K, CX - 0.20f * K, CY + 0.34f * K, U(6.0f), Steel);
	Line(CX - 0.20f * K, CY + 0.02f * K, CX + 0.26f * K, CY - 0.34f * K, U(6.0f), Amber);
	Line(CX - 0.06f * K, CY - 0.10f * K, CX + 0.28f * K, CY + 0.34f * K, U(6.0f), Steel);
	// corner ticks
	Box(CX - U(1.5f), CY - R - U(14.0f), U(3.0f), U(8.0f), Dim);
	Box(CX - U(1.5f), CY + R + U(6.0f), U(3.0f), U(8.0f), Dim);
}

void ARRHUD::PollInput()
{
	PressedKeys.Reset();
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	static const FKey Tracked[] = {
		EKeys::Up, EKeys::Down, EKeys::Left, EKeys::Right, EKeys::W, EKeys::A, EKeys::S, EKeys::D,
		EKeys::Enter, EKeys::SpaceBar, EKeys::Escape, EKeys::BackSpace, EKeys::X, EKeys::Q, EKeys::E, EKeys::Tab,
		EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::AnyKey };
	for (const FKey& K : Tracked)
	{
		if (K == EKeys::AnyKey) continue;
		const bool bDown0 = PC->IsInputKeyDown(K);
		bool& bPrev = PrevKeyDown.FindOrAdd(K);
		if (bDown0 && !bPrev) PressedKeys.Add(K);
		bPrev = bDown0;
	}

	PC->GetMousePosition(MouseX, MouseY);
	bClickL = KeyPressed(EKeys::LeftMouseButton);
	bClickR = KeyPressed(EKeys::RightMouseButton);

	bUp = KeyPressed(EKeys::Up) || KeyPressed(EKeys::W);
	bDown = KeyPressed(EKeys::Down) || KeyPressed(EKeys::S);
	bLeft = KeyPressed(EKeys::Left) || KeyPressed(EKeys::A);
	bRight = KeyPressed(EKeys::Right) || KeyPressed(EKeys::D);
	bConfirm = KeyPressed(EKeys::Enter) || KeyPressed(EKeys::SpaceBar);
	bBack = KeyPressed(EKeys::Escape) || KeyPressed(EKeys::BackSpace);
}

bool ARRHUD::Hit(float X, float Y, float W, float H) const
{
	return MouseX >= X && MouseX <= X + W && MouseY >= Y && MouseY <= Y + H;
}
