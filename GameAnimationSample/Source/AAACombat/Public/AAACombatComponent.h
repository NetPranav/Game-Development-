// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "AAACombatComponent.generated.h"

class UAAAWeaponSettings;
class AAAAWeaponActor;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FAAAHitReactionSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Front;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Stagger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA|Combat|HitReactions")
	TObjectPtr<UAnimMontage> Death;
};

UENUM(BlueprintType)
enum class EAAAAttackType : uint8
{
	Light,
	Heavy
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FAAAOnHitConfirmed, AActor*, Attacker, AActor*, Victim, const FHitResult&, Hit, bool, bHeavy, float, Damage);

UENUM(BlueprintType)
enum class EAAACombatStance : uint8
{
	Unarmed				UMETA(DisplayName = "Unarmed"),
	Armed_Holstered		UMETA(DisplayName = "Armed (Holstered)"),
	Armed_Drawn			UMETA(DisplayName = "Armed (Drawn)")
};

UCLASS( ClassGroup=(AAA), meta=(BlueprintSpawnableComponent) )
class AAACOMBAT_API UAAACombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAACombatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA|Combat|HitReactions")
	FAAAHitReactionSet HitReactions;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Equips a weapon based on the provided settings */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void EquipWeapon(UAAAWeaponSettings* NewWeaponSettings);

	/** Draws the weapon from holster into hand */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void DrawWeapon();

	/** Sheaths the weapon from hand back to holster socket */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void SheathWeapon();

	/** Toggles between drawn and sheathed weapon state */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void ToggleWeaponDraw();

	/** Attaches weapon to hand socket (called directly or via AnimNotify_AttachWeapon) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void AttachWeaponToHand();

	/** Attaches weapon to holster socket (called directly or via AnimNotify_AttachWeapon) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void AttachWeaponToHolster();

	/** Re-attaches the weapon for the current stance (call after the visible mesh changed). */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void RefreshWeaponAttachment() { UpdateWeaponAttachment(); }

	/** Returns current combat stance */
	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	EAAACombatStance GetCombatStance() const { return CombatStance; }

	/** Returns whether weapon is currently drawn in hand */
	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	bool IsWeaponDrawn() const { return CombatStance == EAAACombatStance::Armed_Drawn; }

	/** Returns the currently equipped weapon actor */
	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	AAAAWeaponActor* GetCurrentWeapon() const { return CurrentWeapon; }

	/** Returns the currently equipped weapon settings */
	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	UAAAWeaponSettings* GetCurrentWeaponSettings() const { return CurrentWeaponSettings; }

	/** Resets the auto-holster countdown timer after an attack or combat action */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void RefreshCombatTimeout(float TimeoutSeconds = 8.0f);

	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void RequestAttack(EAAAAttackType Type);

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	bool ConsumeBufferedAttack(EAAAAttackType& OutType);

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	EAAAAttackType GetRequestedAttackType() const { return RequestedAttackType; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AAA|Combat")
	void SetLoadoutModifiers(float DamageMult, float DamageReduct);

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	float GetDamageMultiplier() const { return DamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	float GetDamageReduction() const { return DamageReduction; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);

	UPROPERTY(BlueprintAssignable, Category = "AAA|Combat")
	FAAAOnHitConfirmed OnHitConfirmed;

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	UAnimInstance* FindAnimInstance() const;

	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	float PlayMontageOnOwner(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void PlayHitReaction(AActor* Attacker, bool bHeavy);

	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void PlayBlockImpact();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION()
	void OnRep_CombatStance();

	void UpdateWeaponAttachment();
	void UpdateAbilitySystemTags();

protected:
	/** The actual spawned weapon actor */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, Transient)
	TObjectPtr<AAAAWeaponActor> CurrentWeapon;

	/** The settings data asset for the current weapon */
	UPROPERTY(Replicated, Transient)
	TObjectPtr<UAAAWeaponSettings> CurrentWeaponSettings;

	/** Current stance of the character (Unarmed, Holstered, Drawn) */
	UPROPERTY(ReplicatedUsing = OnRep_CombatStance, Transient)
	EAAACombatStance CombatStance = EAAACombatStance::Unarmed;

	/** Timer handle for auto-sheathing weapon after leaving combat */
	FTimerHandle AutoSheathTimerHandle;

	UPROPERTY(Transient)
	EAAAAttackType RequestedAttackType = EAAAAttackType::Light;

	UPROPERTY(Transient)
	EAAAAttackType BufferedAttack = EAAAAttackType::Light;

	UPROPERTY(Transient)
	float BufferExpiresAt = 0.0f;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "AAA|Combat")
	float DamageMultiplier = 1.0f;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "AAA|Combat")
	float DamageReduction = 0.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void SetBlocking(bool bWantsBlock);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetBlocking(bool bWantsBlock);

protected:
	UPROPERTY(Transient)
	bool bIsBlocking = false;

	UPROPERTY(Transient)
	float LastParryWindowStart = -1.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void ApplyTimedState(FGameplayTag Tag, float Seconds);

	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void ClearState(FGameplayTag Tag);

	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	bool HasState(FGameplayTag Tag) const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeRegen();
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnPoiseChanged(const FOnAttributeChangeData& Data);
	void RegenTick();

	void OnParriedEvent(const FGameplayEventData* Payload);
	void OnBlockedEvent(const FGameplayEventData* Payload);
	void OnStaggeredEvent(const FGameplayEventData* Payload);
	void OnDeathEvent(const FGameplayEventData* Payload);
	void OnBlockingTagChanged(const FGameplayTag Tag, int32 NewCount);

	FTimerHandle RegenTimerHandle;
	FTimerHandle InitRetryTimerHandle;
	FDelegateHandle ParriedEventHandle;
	FDelegateHandle BlockedEventHandle;
	FDelegateHandle StaggeredEventHandle;
	FDelegateHandle DeathEventHandle;
	FDelegateHandle BlockingTagEventHandle;
	
	TMap<FGameplayTag, FTimerHandle> StateTimers;

	float LastStaminaDrainTime = 0.0f;
	float LastPoiseDrainTime = 0.0f;
};
