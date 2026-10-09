// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RRTypes.h"
#include "RRLoadoutComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadoutChangedSignature);

USTRUCT(BlueprintType)
struct FRRInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RR|Inventory")
	FRRGearData Gear;

	UPROPERTY(BlueprintReadWrite, Category = "RR|Inventory")
	bool bOnChain = false;

	UPROPERTY(BlueprintReadWrite, Category = "RR|Inventory")
	int32 VisualVariant = 0;
};

UCLASS(ClassGroup=(RR), meta=(BlueprintSpawnableComponent))
class RRGAME_API URRLoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URRLoadoutComponent();

	UFUNCTION(BlueprintCallable, Category = "RR|Inventory")
	TArray<FRRInventoryItem> BuildInventory() const;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "RR|Loadout")
	void Equip(const FRRGearData& GearData);

	UFUNCTION(BlueprintCallable, Category = "RR|Loadout")
	void Unequip(ERRGearKind Kind);

	UFUNCTION(BlueprintCallable, Category = "RR|Loadout")
	bool GetEquipped(ERRGearKind Kind, FRRGearData& OutGear) const;

	UFUNCTION(BlueprintPure, Category = "RR|Loadout")
	int64 GetVisualSeed() const;

	void UpdateShieldAttachment();

	UPROPERTY(BlueprintAssignable, Category = "RR|Loadout")
	FOnLoadoutChangedSignature OnLoadoutChanged;

protected:
	virtual void BeginPlay() override;

	void ApplyStats();

	UFUNCTION()
	void OnRep_Loadout();

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	FRRGearData Weapon;

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	bool bHasWeapon = false;

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	FRRGearData Armor;

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	bool bHasArmor = false;

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	FRRGearData Ability;

	UPROPERTY(ReplicatedUsing=OnRep_Loadout, Transient)
	bool bHasAbility = false;

	UPROPERTY(Transient)
	float BaseMaxHealth = -1.0f;

	UPROPERTY(Transient)
	TObjectPtr<class UAAAWeaponSettings> RuntimeWeaponSettings;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ShieldMeshComponent;
};
