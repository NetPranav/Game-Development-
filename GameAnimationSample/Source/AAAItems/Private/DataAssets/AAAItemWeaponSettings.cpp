// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "DataAssets/AAAItemWeaponSettings.h"

UAAAItemWeaponSettings::UAAAItemWeaponSettings()
{
	Category = EAAAItemCategory::Weapon;
	bHasDurability = true;
	EquipmentSlot = EAAAEquipmentSlot::MainWeapon;
}
