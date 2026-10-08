// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Effects/AAAGE_StaminaCost.h"
#include "AAAAttributeSet.h"
#include "AAAGameplayTags.h"

UAAAGE_StaminaCost::UAAAGE_StaminaCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo StaminaMod;
	StaminaMod.Attribute = UAAAAttributeSet::GetStaminaAttribute();
	StaminaMod.ModifierOp = EGameplayModOp::Additive;
	FSetByCallerFloat SetByCallerStamina;
	SetByCallerStamina.DataTag = FAAAGameplayTags::Get().Data_StaminaCost;
	StaminaMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerStamina);
	Modifiers.Add(StaminaMod);
}
