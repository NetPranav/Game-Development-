// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#include "RRSuiBridge.h"
#include "RRGame.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformProcess.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "RRSettings.h"

namespace
{
	/** Serializes a flat string/bool/number JSON object with proper escaping. */
	FString MakeJsonBody(const TFunctionRef<void(FJsonObject&)>& Fill)
	{
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Fill(*Obj);
		FString Out;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
		FJsonSerializer::Serialize(Obj, Writer);
		return Out;
	}
}

URRSuiBridge::URRSuiBridge()
{
}

void URRSuiBridge::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const URRSettings* Settings = URRSettings::Get())
	{
		BaseUrl = Settings->SuiSidecarUrl;
	}

	TestCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("rr.sui.test"),
		TEXT("Test Sui Bridge"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([this](const TArray<FString>& Args, UWorld* World)
		{
			UE_LOG(LogRR, Log, TEXT("Running rr.sui.test..."));
			CheckHealth();
			BeginWalletConnect();
			FTimerHandle TimerHandle;
			if (World)
			{
				World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() {
					if (IsConnected())
					{
						FetchGear();
					}
				}), 2.0f, false);
			}
		}),
		ECVF_Default
	);
}

void URRSuiBridge::Deinitialize()
{
	CancelWalletConnect();
	if (TestCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(TestCommand);
		TestCommand = nullptr;
	}
	Super::Deinitialize();
}

void URRSuiBridge::SendRequest(const FString& Verb, const FString& Path, const FString& OptionalBodyJson, TFunction<void(bool bOk, TSharedPtr<FJsonObject>)> Callback)
{
	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		Callback(false, nullptr);
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(BaseUrl + Path);
	Request->SetVerb(Verb);
	Request->SetTimeout(15.0f);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	if (!OptionalBodyJson.IsEmpty())
	{
		Request->SetContentAsString(OptionalBodyJson);
	}

	TWeakObjectPtr<URRSuiBridge> WeakThis(this);

	Request->OnProcessRequestComplete().BindLambda([WeakThis, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		if (!bConnectedSuccessfully || !Res.IsValid())
		{
			if (!WeakThis->bSuiUnreachableLogged)
			{
				WeakThis->bSuiUnreachableLogged = true;
				WeakThis->OnBridgeError.Broadcast(TEXT("Sui sidecar unreachable (is it running on :3001?)"));
			}
			Callback(false, nullptr);
			return;
		}

		WeakThis->bSuiUnreachableLogged = false; // Reset on success

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			bool bOk = false;
			if (JsonObject->TryGetBoolField(TEXT("ok"), bOk))
			{
				if (!bOk)
				{
					FString ErrorMsg = TEXT("Unknown error");
					JsonObject->TryGetStringField(TEXT("error"), ErrorMsg);
					WeakThis->OnBridgeError.Broadcast(ErrorMsg);
					Callback(false, JsonObject);
					return;
				}
			}
			
			if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
			{
				Callback(true, JsonObject);
			}
			else
			{
				FString ErrorMsg = TEXT("HTTP Error");
				JsonObject->TryGetStringField(TEXT("error"), ErrorMsg);
				WeakThis->OnBridgeError.Broadcast(ErrorMsg);
				Callback(false, JsonObject);
			}
		}
		else
		{
			WeakThis->OnBridgeError.Broadcast(TEXT("Invalid JSON response"));
			Callback(false, nullptr);
		}
	});

	Request->ProcessRequest();
}

void URRSuiBridge::CheckHealth()
{
	SendRequest(TEXT("GET"), TEXT("/health"), TEXT(""), [this](bool bOk, TSharedPtr<FJsonObject> Json)
	{
		if (bOk)
		{
			UE_LOG(LogRR, Log, TEXT("Sui Bridge health OK."));
		}
	});
}

void URRSuiBridge::BeginWalletConnect()
{
	FPlatformProcess::LaunchURL(*(BaseUrl + TEXT("/connect")), nullptr, nullptr);
	
	CancelWalletConnect();
	WalletPollCount = 0;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(WalletPollTimer, this, &URRSuiBridge::PollWalletSession, 1.0f, true);
	}
}

void URRSuiBridge::PollWalletSession()
{
	if (WalletPollCount >= 120)
	{
		CancelWalletConnect();
		return;
	}

	WalletPollCount++;

	SendRequest(TEXT("GET"), TEXT("/session"), TEXT(""), [this](bool bOk, TSharedPtr<FJsonObject> Json)
	{
		if (bOk && Json.IsValid())
		{
			bool bConnected = false;
			if (Json->TryGetBoolField(TEXT("connected"), bConnected) && bConnected)
			{
				FString Address;
				if (Json->TryGetStringField(TEXT("address"), Address))
				{
					bIsConnected = true;
					WalletAddress = Address;
					CancelWalletConnect();
					OnWalletConnected.Broadcast(WalletAddress);
					UE_LOG(LogRR, Log, TEXT("Wallet connected: %s"), *WalletAddress);
				}
			}
		}
	});
}

void URRSuiBridge::CancelWalletConnect()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WalletPollTimer);
	}
}

void URRSuiBridge::FetchGear()
{
	if (!bIsConnected || WalletAddress.IsEmpty())
	{
		OnBridgeError.Broadcast(TEXT("Not connected to wallet."));
		return;
	}

	FString Path = FString::Printf(TEXT("/gear/%s"), *WalletAddress);
	SendRequest(TEXT("GET"), Path, TEXT(""), [this](bool bOk, TSharedPtr<FJsonObject> Json)
	{
		if (bOk && Json.IsValid())
		{
			OwnedGear.Empty();
			const TArray<TSharedPtr<FJsonValue>>* GearArray;
			if (Json->TryGetArrayField(TEXT("gear"), GearArray))
			{
				for (const TSharedPtr<FJsonValue>& Val : *GearArray)
				{
					if (Val->Type == EJson::Object)
					{
						OwnedGear.Add(FRRGearData::FromJson(Val->AsObject()));
					}
				}
			}
			
			double CreditsVal = 0;
			if (Json->TryGetNumberField(TEXT("credits"), CreditsVal))
			{
				Credits = (int32)CreditsVal;
			}
			
			OnGearFetched.Broadcast(OwnedGear);
		}
	});
}

void URRSuiBridge::RequestReward(bool bWon, int32 EnemiesKilled)
{
	if (!bIsConnected || WalletAddress.IsEmpty())
	{
		OnBridgeError.Broadcast(TEXT("Not connected to wallet."));
		return;
	}

	const FString BodyJson = MakeJsonBody([&](FJsonObject& O)
	{
		O.SetStringField(TEXT("address"), WalletAddress);
		O.SetBoolField(TEXT("won"), bWon);
		O.SetNumberField(TEXT("enemiesKilled"), EnemiesKilled);
	});

	SendRequest(TEXT("POST"), TEXT("/reward"), BodyJson, [this](bool bOk, TSharedPtr<FJsonObject> Json)
	{
		FRRRewardResult Result;
		Result.bOk = bOk;
		if (Json.IsValid()) // Even on error we might have a result
		{
			Json->TryGetStringField(TEXT("digest"), Result.Digest);
			Json->TryGetStringField(TEXT("url"), Result.Url);
			Json->TryGetStringField(TEXT("error"), Result.Error);
			
			double CreditsAdded = 0;
			if (Json->TryGetNumberField(TEXT("creditsAdded"), CreditsAdded))
			{
				Result.CreditsAdded = (int32)CreditsAdded;
			}

			const TSharedPtr<FJsonObject>* MintedObj;
			if (Json->TryGetObjectField(TEXT("minted"), MintedObj) && MintedObj->IsValid())
			{
				Result.Minted = FRRGearData::FromJson(*MintedObj);
				Result.bHasMinted = true;
			}

		}
		if (!bOk && Result.Error.IsEmpty())
		{
			Result.Error = TEXT("Reward request failed");
		}
		// Always broadcast so the HUD can leave its "minting..." state on failure too.
		OnRewardResult.Broadcast(Result);
	});
}

void URRSuiBridge::UpgradeWeapon(FString WeaponId)
{
	if (!bIsConnected || WalletAddress.IsEmpty())
	{
		OnBridgeError.Broadcast(TEXT("Not connected to wallet."));
		return;
	}

	const FString BodyJson = MakeJsonBody([&](FJsonObject& O)
	{
		O.SetStringField(TEXT("address"), WalletAddress);
		O.SetStringField(TEXT("weaponId"), WeaponId);
	});

	SendRequest(TEXT("POST"), TEXT("/upgrade"), BodyJson, [this](bool bOk, TSharedPtr<FJsonObject> Json)
	{
		if (!bOk)
		{
			UE_LOG(LogRR, Warning, TEXT("UpgradeWeapon returned an error (likely 501 Not Implemented)."));
		}
		// The prompt mentioned "/upgrade implemented but handles the 501 error gracefully."
		// The error will be broadcast by SendRequest automatically.
	});
}
