// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Effects/AAAGE_Damage.h"
#include "AAAAttributeSet.h"
#include "AAAGameplayTags.h"

UAAAGE_Damage::UAAAGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo DamageMod;
	DamageMod.Attribute = UAAAAttributeSet::GetDamageAttribute();
	DamageMod.ModifierOp = EGameplayModOp::Additive;
	FSetByCallerFloat SetByCallerDamage;
	SetByCallerDamage.DataTag = FAAAGameplayTags::Get().Data_Damage;
	DamageMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDamage);
	Modifiers.Add(DamageMod);

	FGameplayModifierInfo PoiseMod;
	PoiseMod.Attribute = UAAAAttributeSet::GetIncomingPoiseDamageAttribute();
	PoiseMod.ModifierOp = EGameplayModOp::Additive;
	FSetByCallerFloat SetByCallerPoise;
	SetByCallerPoise.DataTag = FAAAGameplayTags::Get().Data_PoiseDamage;
	PoiseMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerPoise);
	Modifiers.Add(PoiseMod);
}
