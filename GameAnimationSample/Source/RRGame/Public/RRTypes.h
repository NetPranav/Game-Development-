// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "RRTypes.generated.h"

UENUM(BlueprintType)
enum class ERRGearKind : uint8
{
	Weapon,
	Armor,
	Ability
};

USTRUCT(BlueprintType)
struct FRRGearData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	FString Id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	ERRGearKind Kind = ERRGearKind::Weapon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int64 Seed = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int32 Damage = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int32 Armor = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int32 Power = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Gear")
	int32 Wins = 0;

	static FRRGearData FromJson(const TSharedPtr<FJsonObject>& JsonObject)
	{
		FRRGearData Data;
		if (JsonObject.IsValid())
		{
			FString KindStr;
			JsonObject->TryGetStringField(TEXT("id"), Data.Id);
			JsonObject->TryGetStringField(TEXT("kind"), KindStr);
			if (KindStr == TEXT("weapon")) Data.Kind = ERRGearKind::Weapon;
			else if (KindStr == TEXT("armor")) Data.Kind = ERRGearKind::Armor;
			else if (KindStr == TEXT("ability")) Data.Kind = ERRGearKind::Ability;

			JsonObject->TryGetStringField(TEXT("name"), Data.Name);
			FString SeedStr;
			if (JsonObject->TryGetStringField(TEXT("seed"), SeedStr))
			{
				Data.Seed = FCString::Atoi64(*SeedStr);
			}

			double OutVal = 0;
			if (JsonObject->TryGetNumberField(TEXT("damage"), OutVal)) Data.Damage = (int32)OutVal;
			if (JsonObject->TryGetNumberField(TEXT("armor"), OutVal)) Data.Armor = (int32)OutVal;
			if (JsonObject->TryGetNumberField(TEXT("power"), OutVal)) Data.Power = (int32)OutVal;
			if (JsonObject->TryGetNumberField(TEXT("level"), OutVal)) Data.Level = (int32)OutVal;
			if (JsonObject->TryGetNumberField(TEXT("wins"), OutVal)) Data.Wins = (int32)OutVal;
		}
		return Data;
	}

	FString ToDisplayString() const
	{
		FString StatStr;
		switch (Kind)
		{
		case ERRGearKind::Weapon: StatStr = FString::Printf(TEXT("DMG %d"), Damage); break;
		case ERRGearKind::Armor: StatStr = FString::Printf(TEXT("ARM %d"), Armor); break;
		case ERRGearKind::Ability: StatStr = FString::Printf(TEXT("PWR %d"), Power); break;
		}
		return FString::Printf(TEXT("%s  %s  Lv%d"), *Name, *StatStr, Level);
	}
};

USTRUCT(BlueprintType)
struct FRRRewardResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	bool bOk = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	FString Digest;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	FString Url;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	FString Error;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	FRRGearData Minted;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	bool bHasMinted = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AAA|Reward")
	int32 CreditsAdded = 0;
};
