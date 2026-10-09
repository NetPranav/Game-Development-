// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RREnemy.h"
#include "RRBoss.generated.h"

class UAnimSequence;
class UAnimMontage;
class AAAAWeaponActor;

struct FGameplayEventData;

UENUM(BlueprintType)
enum class ERRBossAttackKind : uint8
{
	Normal,
	PerilousThrust,
	PerilousSweep,
	PerilousGrab,
	PerilousSpecial
};

USTRUCT(BlueprintType)
struct FRRBossAttack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	TSoftObjectPtr<UAnimSequence> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float PoiseDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float WindupSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float HitWindowStart = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float HitWindowEnd = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float Range = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	int32 MinPhase = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	ERRBossAttackKind Kind = ERRBossAttackKind::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	bool bCinematic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float RecoverySeconds = 1.0f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FRRBossDeathblowNative, ARRBoss* /*Boss*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRRBossPhaseChangedNative, ARRBoss* /*Boss*/, int32 /*Phase*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FRRBossPerilousWarningNative, ARRBoss* /*Boss*/, ERRBossAttackKind /*Kind*/, float /*WarningSeconds*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRRBossMikiriNative, ARRBoss* /*Boss*/, AActor* /*Target*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRRBossSpecialAttackNative, ARRBoss* /*Boss*/, AActor* /*Target*/);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FRRBossSpecialBeginNative, ARRBoss* /*Boss*/, AActor* /*Target*/, float /*SecondsToContact*/);

struct RRGAME_API FRRBossEvents
{
public:
	static FRRBossSpecialBeginNative& OnSpecialAttackBegin();
	static FRRBossDeathblowNative& OnDeathblow();
	static FRRBossPhaseChangedNative& OnPhaseChanged();
	static FRRBossPerilousWarningNative& OnPerilousWarning();
	static FRRBossMikiriNative& OnMikiri();
	static FRRBossSpecialAttackNative& OnSpecialAttackContact();
};

UCLASS()
class RRGAME_API ARRBoss : public ARREnemy
{
	GENERATED_BODY()

public:
	ARRBoss(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	int32 HealthSegments = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float SegmentMaxHealth = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float MaxPosture = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float PostureRegenDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float StaggerSeconds = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float BrokenSegmentStaggerSeconds = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	TArray<FRRBossAttack> Attacks;

	UPROPERTY(ReplicatedUsing=OnRep_SegmentsRemaining, BlueprintReadOnly, Category = "RR|Boss")
	int32 SegmentsRemaining;

	UPROPERTY(ReplicatedUsing=OnRep_Phase, BlueprintReadOnly, Category = "RR|Boss")
	int32 Phase = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "RR|Boss")
	bool bDeathblowReady = false;

	UFUNCTION()
	void OnRep_SegmentsRemaining();

	UFUNCTION()
	void OnRep_Phase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ExecuteDeathblow(AActor* Attacker);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeathblowPresentation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPhaseChangedPresentation(int32 NewPhase);

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	int32 GetSegmentsRemaining() const { return SegmentsRemaining; }

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	float GetSegmentFraction() const;

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	float GetPostureFraction() const;

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	bool IsPostureBroken() const;

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	bool IsDeathblowReady() const { return bDeathblowReady; }

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	int32 GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	FText GetBossName() const { return BossName; }

	/** True while it lies on the ground / rises / taunts after losing a health segment (AI paused, invulnerable). */
	UFUNCTION(BlueprintPure, Category = "RR|Boss")
	bool IsInPhaseTransition() const { return bPhaseTransition; }

	/** Plays a Greystone animation by name on the full-body slot; returns its length. */
	float PlayBossAnim(const TCHAR* AnimName, float Rate = 1.0f, float BlendIn = 0.2f, float BlendOut = 0.3f, bool bHoldLastFrame = false);

	/** Sword-point / "bring it on" taunt (match start). */
	void PlayTaunt();

	/** Seconds it stays down after a deathblow before rising. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float PhaseLieSeconds = 1.6f;

	/** Yaw turn rates (deg/s): normal, while winding up an attack (tracks the player), after the swing commits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float DefaultTurnRate = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float TrackingTurnRate = 480.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Boss")
	float CommittedTurnRate = 45.0f;

protected:
	UFUNCTION()
	void OnHitConfirmedHandler(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);

	UFUNCTION()
	void OnStaggeredHandler(AActor* Victim);

	UFUNCTION()
	void OnDeathHandler();

	void OnWeaponHit(const FGameplayEventData* Payload);

public:
	void PlayAttackMontage(const FRRBossAttack& Attack);
protected:

	void BeginPhaseTransition();
	void SetTurnRate(float YawDegPerSec);
	bool bPhaseTransition = false;
	FTimerHandle PhaseTimer;

	FTimerHandle PostureRegenTimer;
	void RegenPostureTick();

	FDelegateHandle HitConfirmedHandle;
	FDelegateHandle StaggeredHandle;

	float CurrentAttackDamageMult = 1.0f;
	float CurrentAttackPoiseDamage = 0.0f;
	ERRBossAttackKind CurrentAttackKind = ERRBossAttackKind::Normal;

	FTimerHandle TraceStartHandle;
	FTimerHandle TraceEndHandle;
	FTimerHandle AttackRecoverHandle;
	FTimerHandle PerilousWarningHandle;
};
