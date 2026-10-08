// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AAACombatSettings.generated.h"

/**
 * Global combat tuning settings.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="AAA Combat"))
class AAACOMBAT_API UAAACombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAAACombatSettings();

	static const UAAACombatSettings* Get();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float BlockDamageReduction;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float BlockStaminaPerDamage;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float ParryWindowSeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float ParryStaggerSeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float StaggerSeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float PoiseRegenDelay;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float PoiseRegenPerSecond;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float StaminaRegenPerSecond;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float LightPoiseDamage;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HeavyPoiseDamage;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HeavyDamageMultiplier;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float LightStaminaCost;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HeavyStaminaCost;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float DodgeStaminaCost;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HitStopLightSeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HitStopHeavySeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float ComboInputBufferSeconds;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float LockOnRange;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float LockOnMaxAngleDegrees;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float PoiseBrokenDamageMultiplier;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float KnockbackLight;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float KnockbackHeavy;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float KnockbackStagger;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float HitReactMinDamageFraction;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game")
	bool bEnableHitStop;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game")
	bool bEnableCameraShake;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float CameraShakeLightScale;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0.0"))
	float CameraShakeHeavyScale;

	// ---- Combat feel (combo, aim assist, timing) ----

	/** Damage multiplier per light-combo step (click 1, 2, 3...). The last step is the "finisher". */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel")
	TArray<float> ComboDamageMultipliers;

	/** The last light-combo step also counts as a heavy hit for feedback and deals extra poise damage by this factor. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "1.0"))
	float FinisherPoiseMultiplier;

	/** Seconds without a click after which the combo restarts from the first attack. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.2"))
	float ComboResetSeconds;

	/** When a player starts an attack the character turns toward the best enemy within this range (cm)... */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0"))
	float AttackAssistRange;

	/** ...and within this cone half-angle (degrees) of the facing (or movement-input) direction. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AttackAssistAngleDegrees;

	/** Forward lunge speed (cm/s) and duration (s) applied at the start of each player attack so swings connect. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0"))
	float LungeSpeed;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0"))
	float LungeDuration;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "1.0"))
	float FinisherLungeMultiplier;

	/** Fractions of an attack montage during which the next click may chain, and the weapon trace is live. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ComboWindowOpenFraction;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ComboWindowCloseFraction;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TraceOpenFraction;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Feel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TraceCloseFraction;

	/** Damage multiplier for the combo step, clamped to the last entry. */
	float GetComboDamageMultiplier(int32 Step) const
	{
		if (ComboDamageMultipliers.Num() == 0) return 1.0f;
		return ComboDamageMultipliers[FMath::Clamp(Step, 0, ComboDamageMultipliers.Num() - 1)];
	}
};
