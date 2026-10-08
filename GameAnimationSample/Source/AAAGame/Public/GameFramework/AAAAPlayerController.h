// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AAAAPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockTargetChanged, AActor*, NewTarget);

/**
 * AAAAPlayerController
 * 
 * The main input and camera orchestrator for the local player.
 * 
 * Responsibilities:
 * - Enhanced Input initialization and routing.
 * - Camera ownership (especially during transitions or cutscenes).
 * - UI management (toggling pause menus, inventory, dialogue).
 * - Target Lock-on systems.
 * - Future RTS / Strategic camera controls.
 * 
 * Note: No gameplay logic (like damage calculation) should exist here.
 */
UCLASS()
class AAAGAME_API AAAAAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAAAAPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	//-------------------------------------------------------------------------
	// Input & UI Management
	//-------------------------------------------------------------------------
	
	/** Global mapping context for UI and system controls (e.g. Pause Menu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Input")
	TObjectPtr<class UInputMappingContext> GlobalUIMappingContext;

	/** Action mapped to the Tab key (or Start button) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA|Input")
	TObjectPtr<class UInputAction> ToggleMenuAction;
	
	/** Called when the player presses the Menu button */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnToggleMenu();

	/** Closes the menu if it is currently open */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void CloseMenuIfOpen();

	/** Called when the player presses the Draw/Sheath button (R) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnDrawSheathWeapon();

	/** Called when the player presses Attack (LMB) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnAttackAction();

	/** Called when the player presses Heavy Attack (F) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnHeavyAttackAction();

	/** Called when the player presses Dodge (Shift) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnDodgeAction();

	/** Called when the player presses Block (RMB) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnBlockPressed();

	/** Called when the player releases Block (RMB) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Input")
	void OnBlockReleased();

	virtual void OnUnPossess() override;

	//-------------------------------------------------------------------------
	// Target Lock-On
	//-------------------------------------------------------------------------

public:
	/** Returns the currently locked target, or nullptr if none */
	UFUNCTION(BlueprintPure, Category = "AAA|Combat")
	AActor* GetLockedTarget() const { return LockedTarget; }

	/** Fired when the locked target changes */
	UPROPERTY(BlueprintAssignable, Category = "AAA|Combat")
	FOnLockTargetChanged OnLockTargetChanged;

protected:
	/** Toggles lock-on state */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void ToggleLockOn();

	/** Cycles the lock-on target left (-1) or right (1) */
	UFUNCTION(BlueprintCallable, Category = "AAA|Combat")
	void CycleLockOn(int32 Dir);

	void CycleLockOnLeft();
	void CycleLockOnRight();

	void ClearLock();
	TArray<AActor*> GetLockOnCandidates() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LockedTarget;

	float TimeWithoutLOS = 0.0f;
	float TimeLookInputFlick = 0.0f;
};
