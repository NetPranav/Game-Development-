// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAADamageExecutionCalc.h"
#include "AAAAttributeSet.h"
#include "AbilitySystemComponent.h"

// Declare the attributes to capture
struct FAAADamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	FAAADamageStatics()
	{
		// Capture the Target's Armor
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAAAAttributeSet, Armor, Target, false);
		
		// Capture the Source's Base Damage. We assume the Gameplay Effect applying the damage modifies this Meta Attribute.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAAAAttributeSet, Damage, Source, true);
	}
};

static const FAAADamageStatics& DamageStatics()
{
	static FAAADamageStatics DStatics;
	return DStatics;
}

UAAADamageExecutionCalc::UAAADamageExecutionCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
}

void UAAADamageExecutionCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather tags
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Capture Base Damage
	float Damage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters, Damage);

	// Capture Target Armor
	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.0f);

	// Basic mitigation formula: Damage * (100 / (100 + Armor))
	float MitigatedDamage = Damage;
	if (Armor > 0.0f)
	{
		MitigatedDamage = Damage * (100.0f / (100.0f + Armor));
	}

	if (MitigatedDamage > 0.0f)
	{
		// Apply the calculated damage to the Target's Damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	}
}
