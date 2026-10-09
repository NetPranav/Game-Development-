// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAACombatSettings.h"

UAAACombatSettings::UAAACombatSettings()
{
	BlockDamageReduction = 0.85f;
	BlockStaminaPerDamage = 0.5f;
	ParryWindowSeconds = 0.35f;
	ParryStaggerSeconds = 1.6f;
	StaggerSeconds = 0.9f;
	PoiseRegenDelay = 3.0f;
	PoiseRegenPerSecond = 20.0f;
	StaminaRegenDelay = 1.0f;
	StaminaRegenPerSecond = 28.0f;
	LightPoiseDamage = 15.0f;
	HeavyPoiseDamage = 45.0f;
	HeavyDamageMultiplier = 1.8f;
	LightStaminaCost = 12.0f;
	HeavyStaminaCost = 28.0f;
	DodgeStaminaCost = 15.0f;
	HitStopLightSeconds = 0.045f;
	HitStopHeavySeconds = 0.11f;
	ComboInputBufferSeconds = 0.80f;
	LockOnRange = 1600.0f;
	LockOnMaxAngleDegrees = 65.0f;
	PoiseBrokenDamageMultiplier = 1.25f;

	ComboDamageMultipliers = { 1.0f, 1.15f, 1.35f, 1.55f, 2.0f };
	FinisherPoiseMultiplier = 2.0f;
	ComboResetSeconds = 1.4f;
	AttackAssistRange = 380.0f;
	AttackAssistAngleDegrees = 70.0f;
	LungeSpeed = 520.0f;
	LungeDuration = 0.14f;
	FinisherLungeMultiplier = 1.4f;
	ComboWindowOpenFraction = 0.20f;
	ComboWindowCloseFraction = 0.95f;
	TraceOpenFraction = 0.22f;
	TraceCloseFraction = 0.60f;
	KnockbackLight = 250.0f;
	KnockbackHeavy = 450.0f;
	KnockbackStagger = 600.0f;
	HitReactMinDamageFraction = 0.06f;
	bEnableHitStop = true;
	bEnableCameraShake = true;
	CameraShakeLightScale = 1.0f;
	CameraShakeHeavyScale = 2.0f;
}

const UAAACombatSettings* UAAACombatSettings::Get()
{
	return GetDefault<UAAACombatSettings>();
}
