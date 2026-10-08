// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAItemDefinition.h"

UAAAItemDefinition::UAAAItemDefinition()
{
}

FPrimaryAssetId UAAAItemDefinition::GetPrimaryAssetId() const
{
	// Format: "AAAItem:<ItemID>"
	// This allows the Asset Manager to discover and load items by type.
	return FPrimaryAssetId(FPrimaryAssetType("AAAItem"), GetFName());
}
