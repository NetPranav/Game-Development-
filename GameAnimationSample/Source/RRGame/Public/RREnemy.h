// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "RREnemy.generated.h"

class UAAAAbilitySystemComponent;
class UAAAAttributeSet;
class UAAAAttributeComponent;
class UAAATeamComponent;
class UAAACombatComponent;
class UAAAWeaponSettings;

USTRUCT(BlueprintType)
struct FRREnemyTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float MaxHealth = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float MaxPoise = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float Armor = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float WalkSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float ChaseSpeed = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float AggroRange = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float LoseRange = 3600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float AttackRange = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float PreferredCircleRange = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float AttackCooldownMin = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float AttackCooldownMax = 2.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float LightVsHeavy = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float SidestepChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float BlockChanceWhenPlayerAttacking = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	float DespawnAfterDeathSeconds = 6.0f;
};

UCLASS()
class RRGAME_API ARREnemy : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ARREnemy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable, Category = "RR|Enemy")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "RR|Enemy")
	AActor* GetCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "RR|Enemy")
	float GetHealthFraction() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	FRREnemyTuning Tuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Enemy")
	TObjectPtr<UAAAWeaponSettings> WeaponSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAAAAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAAAAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAAAAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAAATeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAAACombatComponent> CombatComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnDeath();

	void StopIntroMontage();

	/** The Paragon animation blueprint's locomotion state machine never leaves its start pose for AI-driven characters,
	 *  so idle/jog loops are played through the DefaultSlot whenever no action montage (attack, hit reaction...) is active. */
	void UpdateLocomotion();
	FTimerHandle LocomotionTimer;
	UPROPERTY(Transient) TObjectPtr<class UAnimMontage> LocoMontage;
	FName LocoCurrent;
	FTimerHandle IntroStopTimer;
	int32 IntroStopTicks = 0;
};
