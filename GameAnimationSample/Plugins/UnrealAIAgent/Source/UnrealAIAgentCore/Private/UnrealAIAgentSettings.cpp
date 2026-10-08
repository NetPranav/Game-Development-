// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "UnrealAIAgentSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UUnrealAIAgentSettings::UUnrealAIAgentSettings()
	: NIMApiKey(TEXT("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"))
	, NIMEndpoint(TEXT("https://integrate.api.nvidia.com/v1/chat/completions"))
	, ModelName(TEXT("nvidia/nemotron-3-ultra-550b-a55b"))
	, Temperature(0.7f)
	, TopP(0.95f)
	, MaxTokens(16384)
	, bEnableThinking(true)
	, MaxSteps(25)
	, MaxRepairAttempts(5)
	, RequestTimeout(600.0f)
	, bDebugLogging(true)
	, bEnableTransactions(true)
	, SystemPromptOverride(TEXT(""))
{
}

FString UUnrealAIAgentSettings::GetEffectiveApiKey() const
{
	if (!NIMApiKey.IsEmpty())
	{
		return NIMApiKey.TrimStartAndEnd();
	}

	// Fallback to environment variable
	FString EnvKey = FPlatformMisc::GetEnvironmentVariable(TEXT("NVIDIA_NIM_API_KEY"));
	if (!EnvKey.IsEmpty())
	{
		return EnvKey.TrimStartAndEnd();
	}

	EnvKey = FPlatformMisc::GetEnvironmentVariable(TEXT("OPENAI_API_KEY"));
	if (!EnvKey.IsEmpty())
	{
		return EnvKey.TrimStartAndEnd();
	}

	// Fallback: check .env files in project and plugin dirs
	TArray<FString> CandidatePaths = {
		FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealAIAgent"), TEXT(".env")),
		FPaths::Combine(FPaths::ProjectDir(), TEXT(".env")),
		FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT(".env"))
	};

	for (const FString& EnvPath : CandidatePaths)
	{
		if (FPaths::FileExists(EnvPath))
		{
			TArray<FString> Lines;
			if (FFileHelper::LoadFileToStringArray(Lines, *EnvPath))
			{
				for (const FString& Line : Lines)
				{
					FString Trimmed = Line.TrimStartAndEnd();
					if (Trimmed.StartsWith(TEXT("NVIDIA_NIM_API_KEY=")))
					{
						FString KeyVal = Trimmed.RightChop(19).TrimQuotes();
						if (!KeyVal.IsEmpty())
						{
							return KeyVal;
						}
					}
				}
			}
		}
	}

	return TEXT("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");
}

const UUnrealAIAgentSettings* UUnrealAIAgentSettings::Get()
{
	return GetDefault<UUnrealAIAgentSettings>();
}

UUnrealAIAgentSettings* UUnrealAIAgentSettings::GetMutable()
{
	return GetMutableDefault<UUnrealAIAgentSettings>();
}
