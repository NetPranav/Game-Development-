// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRLoadoutComponent.h"
#include "Net/UnrealNetwork.h"
#include "AAACombatComponent.h"
#include "AAAAttributeComponent.h"
#include "AAAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "RRGameMode.h"
#include "RRSettings.h"
#include "DataAssets/AAAWeaponSettings.h"

#include "RRSuiBridge.h"
#include "Engine/GameInstance.h"

URRLoadoutComponent::URRLoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

TArray<FRRInventoryItem> URRLoadoutComponent::BuildInventory() const
{
	TArray<FRRInventoryItem> Inventory;

	auto AddLocal = [&](ERRGearKind Kind, FString Id, FString Name, int32 Stat, int32 Variant)
	{
		FRRInventoryItem Item;
		Item.Gear.Kind = Kind;
		Item.Gear.Id = Id;
		Item.Gear.Name = Name;
		Item.Gear.Level = 1;
		Item.Gear.Seed = 0;
		if (Kind == ERRGearKind::Weapon) Item.Gear.Damage = Stat;
		else if (Kind == ERRGearKind::Armor) Item.Gear.Armor = Stat;
		else if (Kind == ERRGearKind::Ability) Item.Gear.Power = Stat;
		Item.bOnChain = false;
		Item.VisualVariant = Variant;
		Inventory.Add(Item);
	};

	AddLocal(ERRGearKind::Weapon, TEXT("local_w1"), TEXT("Rustic Blade"), 12, 0);
	AddLocal(ERRGearKind::Weapon, TEXT("local_w2"), TEXT("Dark Sword I"), 18, 0);
	AddLocal(ERRGearKind::Weapon, TEXT("local_w3"), TEXT("Dark Sword II"), 24, 1);
	AddLocal(ERRGearKind::Armor, TEXT("local_a1"), TEXT("Padded Frame"), 8, 0);
	AddLocal(ERRGearKind::Armor, TEXT("local_a2"), TEXT("Plated Chassis"), 20, 0);
	AddLocal(ERRGearKind::Ability, TEXT("local_ab1"), TEXT("Overdrive"), 15, 0);

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (URRSuiBridge* Sui = GI->GetSubsystem<URRSuiBridge>())
		{
			TArray<FRRGearData> Owned = Sui->GetOwnedGear();
			for (const FRRGearData& Gear : Owned)
			{
				FRRInventoryItem Item;
				Item.Gear = Gear;
				Item.bOnChain = true;
				Item.VisualVariant = (Gear.Kind == ERRGearKind::Weapon) ? (Gear.Seed % 2) : 0;
				Inventory.Add(Item);
			}
		}
	}

	return Inventory;
}

void URRLoadoutComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URRLoadoutComponent, Weapon);
	DOREPLIFETIME(URRLoadoutComponent, bHasWeapon);
	DOREPLIFETIME(URRLoadoutComponent, Armor);
	DOREPLIFETIME(URRLoadoutComponent, bHasArmor);
	DOREPLIFETIME(URRLoadoutComponent, Ability);
	DOREPLIFETIME(URRLoadoutComponent, bHasAbility);
}

void URRLoadoutComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		else if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState()))
			{
				ASC = PS_ASI->GetAbilitySystemComponent();
			}
		}

		if (ASC)
		{
			if (const UAAAAttributeSet* AttrSet = Cast<UAAAAttributeSet>(ASC->GetAttributeSet(UAAAAttributeSet::StaticClass())))
			{
				BaseMaxHealth = AttrSet->GetMaxHealth();
			}
		}
	}
}

void URRLoadoutComponent::Equip(const FRRGearData& GearData)
{
	if (!GetOwner()->HasAuthority())
		return;

	switch (GearData.Kind)
	{
		case ERRGearKind::Weapon:
		{
			Weapon = GearData;
			bHasWeapon = true;
			
			if (ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>())
			{
				if (UAAAWeaponSettings* BaseSettings = GM->PlayerWeaponSettings)
				{
					UAAAWeaponSettings* RuntimeSettings = DuplicateObject<UAAAWeaponSettings>(BaseSettings, this);
					
					int32 Variant = (GearData.Seed % 2);
					if (GearData.Id.StartsWith(TEXT("local_")))
					{
						if (GearData.Name == TEXT("Rustic Blade")) Variant = 0;
						else if (GearData.Name == TEXT("Dark Sword II")) Variant = 1;
					}
					
					// Apply Combat BaseDamage formula: 12 + Damage * 0.5
					RuntimeSettings->Combat.BaseDamage = 12.0f + (GearData.Damage * 0.5f);
					
					const URRSettings* Settings = URRSettings::Get();
					if (Settings && Settings->SwordMeshes.IsValidIndex(Variant))
					{
						if (UStaticMesh* Mesh = Settings->SwordMeshes[Variant].LoadSynchronous())
						{
							RuntimeSettings->Visuals.StaticMesh = Mesh;
						}
					}
					
					RuntimeWeaponSettings = RuntimeSettings;
					
					if (UAAACombatComponent* CombatComp = GetOwner()->FindComponentByClass<UAAACombatComponent>())
					{
						bool bWasDrawn = CombatComp->IsWeaponDrawn();
						CombatComp->EquipWeapon(RuntimeSettings);
						if (!bWasDrawn)
						{
							CombatComp->SheathWeapon();
						}
					}
				}
			}
			break;
		}
		case ERRGearKind::Armor:
			Armor = GearData;
			bHasArmor = true;
			break;
		case ERRGearKind::Ability:
			Ability = GearData;
			bHasAbility = true;
			break;
	}

	ApplyStats();
	OnLoadoutChanged.Broadcast();
}

void URRLoadoutComponent::Unequip(ERRGearKind Kind)
{
	if (!GetOwner()->HasAuthority())
		return;

	switch (Kind)
	{
		case ERRGearKind::Weapon:
			Weapon = FRRGearData();
			bHasWeapon = false;
			break;
		case ERRGearKind::Armor:
			Armor = FRRGearData();
			bHasArmor = false;
			break;
		case ERRGearKind::Ability:
			Ability = FRRGearData();
			bHasAbility = false;
			break;
	}

	ApplyStats();
	OnLoadoutChanged.Broadcast();
}

bool URRLoadoutComponent::GetEquipped(ERRGearKind Kind, FRRGearData& OutGear) const
{
	switch (Kind)
	{
		case ERRGearKind::Weapon:
			OutGear = Weapon;
			return bHasWeapon;
		case ERRGearKind::Armor:
			OutGear = Armor;
			return bHasArmor;
		case ERRGearKind::Ability:
			OutGear = Ability;
			return bHasAbility;
	}
	return false;
}

int64 URRLoadoutComponent::GetVisualSeed() const
{
	int64 W = bHasWeapon ? Weapon.Seed : 0;
	int64 A = bHasArmor ? Armor.Seed : 0;
	return W ^ A;
}

void URRLoadoutComponent::ApplyStats()
{
	if (!GetOwner()->HasAuthority()) return;

	float DamageMult = 1.0f;
	float DamageReduct = 0.0f;

	int32 WepDmg = bHasWeapon ? Weapon.Damage : 0;
	int32 AbPwr = bHasAbility ? Ability.Power : 0;
	int32 ArmArm = bHasArmor ? Armor.Armor : 0;

	DamageMult = 1.0f + (WepDmg / 100.0f) + (AbPwr / 200.0f);
	DamageReduct = (float)ArmArm / ((float)ArmArm + 100.0f);
	if (DamageReduct > 0.6f) DamageReduct = 0.6f;

	if (UAAACombatComponent* CombatComp = GetOwner()->FindComponentByClass<UAAACombatComponent>())
	{
		CombatComp->SetLoadoutModifiers(DamageMult, DamageReduct);
	}

	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}
	else if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState()))
		{
			ASC = PS_ASI->GetAbilitySystemComponent();
		}
	}

	if (ASC && BaseMaxHealth > 0.0f)
	{
		float CurrentHealth = ASC->GetNumericAttribute(UAAAAttributeSet::GetHealthAttribute());
		float CurrentMax = ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxHealthAttribute());
		float HealthFrac = CurrentMax > 0.0f ? (CurrentHealth / CurrentMax) : 1.0f;

		float NewMax = BaseMaxHealth + (ArmArm * 0.5f);
		ASC->SetNumericAttributeBase(UAAAAttributeSet::GetMaxHealthAttribute(), NewMax);
		ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), NewMax * HealthFrac);
	}
}

void URRLoadoutComponent::OnRep_Loadout()
{
	OnLoadoutChanged.Broadcast();
}
