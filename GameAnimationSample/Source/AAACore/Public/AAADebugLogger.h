// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * Centralized AAA Debug & Action Logging System.
 * Displays high-visibility on-screen notifications for every gameplay action,
 * combat state, ability activation, hit reaction, and UI event.
 */
class FAAADebugLogger
{
public:
	/** Displays a temporary floating debug notification on-screen */
	static void LogAction(const FString& Category, const FString& Message, FColor Color = FColor::Green, float Duration = 3.5f)
	{
		if (GEngine)
		{
			FString Formatted = FString::Printf(TEXT("[%s] %s"), *Category, *Message);
			GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Formatted);
		}
		UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *Category, *Message);
	}

	/** Displays a persistent/updating single-line status on-screen (avoids flooding) */
	static void LogStatus(uint64 Key, const FString& Category, const FString& Message, FColor Color = FColor::Cyan)
	{
		if (GEngine)
		{
			FString Formatted = FString::Printf(TEXT("[%s] %s"), *Category, *Message);
			GEngine->AddOnScreenDebugMessage(Key, 2.5f, Color, Formatted);
		}
	}
};
