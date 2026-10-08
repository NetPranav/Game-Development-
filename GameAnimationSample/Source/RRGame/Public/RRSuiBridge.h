// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RRTypes.h"
#include "Dom/JsonObject.h"
#include "RRSuiBridge.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletConnected, FString, Address);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearFetched, const TArray<FRRGearData>&, Gear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardResult, const FRRRewardResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBridgeError, const FString&, ErrorMessage);

UCLASS(Config=Game)
class RRGAME_API URRSuiBridge : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URRSuiBridge();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void CheckHealth();

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void BeginWalletConnect();

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void FetchGear();

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void RequestReward(bool bWon, int32 EnemiesKilled);

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void UpgradeWeapon(FString WeaponId);

	UFUNCTION(BlueprintCallable, Category = "AAA|Sui")
	void CancelWalletConnect();

	UFUNCTION(BlueprintPure, Category = "AAA|Sui")
	bool IsConnected() const { return bIsConnected; }

	UFUNCTION(BlueprintPure, Category = "AAA|Sui")
	FString GetWalletAddress() const { return WalletAddress; }

	UFUNCTION(BlueprintPure, Category = "AAA|Sui")
	TArray<FRRGearData> GetOwnedGear() const { return OwnedGear; }

	UFUNCTION(BlueprintPure, Category = "AAA|Sui")
	int32 GetCredits() const { return Credits; }

	UPROPERTY(BlueprintAssignable, Category = "AAA|Sui")
	FOnWalletConnected OnWalletConnected;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Sui")
	FOnGearFetched OnGearFetched;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Sui")
	FOnRewardResult OnRewardResult;

	UPROPERTY(BlueprintAssignable, Category = "AAA|Sui")
	FOnBridgeError OnBridgeError;

protected:
	UPROPERTY(Config)
	FString BaseUrl = TEXT("http://127.0.0.1:3001");

private:
	void SendRequest(const FString& Verb, const FString& Path, const FString& OptionalBodyJson, TFunction<void(bool bOk, TSharedPtr<FJsonObject>)> Callback);
	
	void PollWalletSession();

	bool bIsConnected = false;
	FString WalletAddress;
	TArray<FRRGearData> OwnedGear;
	int32 Credits = 0;

	FTimerHandle WalletPollTimer;
	int32 WalletPollCount = 0;
	bool bSuiUnreachableLogged = false;

	IConsoleCommand* TestCommand = nullptr;
};
