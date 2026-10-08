// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAItemInstance.h"
#include "AAAItemDefinition.h"
#include "Net/UnrealNetwork.h"

UAAAItemInstance::UAAAItemInstance()
{
	InstanceID = FGuid::NewGuid();
}

void UAAAItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAAAItemInstance, ItemDefinition);
	DOREPLIFETIME(UAAAItemInstance, InstanceID);
	DOREPLIFETIME(UAAAItemInstance, StackCount);
	DOREPLIFETIME(UAAAItemInstance, CurrentDurability);
	DOREPLIFETIME(UAAAItemInstance, Quality);
	DOREPLIFETIME(UAAAItemInstance, DynamicTags);
}

bool UAAAItemInstance::IsValid() const
{
	return ItemDefinition != nullptr;
}

FText UAAAItemInstance::GetDisplayName() const
{
	return IsValid() ? ItemDefinition->ItemName : FText::FromString(TEXT("Invalid Item"));
}

EAAAItemCategory UAAAItemInstance::GetCategory() const
{
	return IsValid() ? ItemDefinition->Category : EAAAItemCategory::None;
}

bool UAAAItemInstance::CanStack() const
{
	if (!IsValid()) return false;
	return StackCount < ItemDefinition->MaxStackSize;
}

int32 UAAAItemInstance::GetRemainingStackSpace() const
{
	if (!IsValid()) return 0;
	return FMath::Max(0, ItemDefinition->MaxStackSize - StackCount);
}

float UAAAItemInstance::GetTotalWeight() const
{
	if (!IsValid()) return 0.0f;
	return ItemDefinition->Weight * static_cast<float>(StackCount);
}

bool UAAAItemInstance::IsBroken() const
{
	if (!IsValid() || !ItemDefinition->bHasDurability) return false;
	return CurrentDurability <= 0.0f;
}
