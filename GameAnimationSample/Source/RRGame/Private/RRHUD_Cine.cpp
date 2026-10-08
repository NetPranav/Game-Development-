// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Letterbox bars while a special-attack cinematic plays.

#include "RRHUD.h"
#include "RRCinematicDirector.h"
#include "Engine/World.h"

void ARRHUD::DrawCinematicBars()
{
	UWorld* World = GetWorld();
	const URRCinematicDirector* Cine = World ? World->GetSubsystem<URRCinematicDirector>() : nullptr;
	const float A = Cine ? Cine->GetBarAlpha() : 0.0f;
	if (A <= 0.001f) return;
	const float Eased = A * A * (3.0f - 2.0f * A);
	const float H = CanvasH * 0.11f * Eased;
	Box(0.0f, 0.0f, CanvasW, H, FLinearColor::Black);
	Box(0.0f, CanvasH - H, CanvasW, H + 1.0f, FLinearColor::Black);
}
