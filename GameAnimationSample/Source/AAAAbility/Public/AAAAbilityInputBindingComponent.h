// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "AAAAbilityInputBindingComponent.generated.h"

class UAbilitySystemComponent;
class UInputAction;
class UEnhancedInputComponent;

/**
 * FAAAAbilityInputBinding
 *
 * Maps an Enhanced Input Action to a Gameplay Tag.
 * When the input fires, the ASC activates any ability that has this tag
 * in its AbilityTags container. This is the Lyra pattern for 100%
 * data-driven ability binding — designers rebind abilities in the editor,
 * never in C++.
 */
USTRUCT(BlueprintType)
struct FAAAAbilityInputBinding
{
	GENERATED_BODY()

	/** The Enhanced Input Action to listen for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Input")
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** The GameplayTag that bridges this input to an ability. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Input")
	FGameplayTag InputTag;

	/**
	 * If true, releasing the input will cancel the ability.
	 * Used for held actions like Sprint or Channel abilities.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA|Input")
	bool bCancelOnRelease = false;
};

/**
 * UAAAAbilityInputBindingComponent
 *
 * Bridges Enhanced Input → Gameplay Tags → Ability System Component.
 *
 * *** ARCHITECTURAL PATTERN (Lyra) ***
 * Instead of binding inputs directly to ability classes, this component:
 * 1. Listens to Enhanced Input Actions
 * 2. When an action fires, it tells the ASC to activate abilities
 *    matching the corresponding InputTag
 * 3. Designers configure bindings entirely in Data Assets
 *
 * This makes the entire input→ability pipeline data-driven.
 * Adding a new ability never requires C++ changes.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAAABILITY_API UAAAAbilityInputBindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAAAAbilityInputBindingComponent();

	/**
	 * Sets up all input bindings from the provided array.
	 * Call this after EnhancedInput subsystem is ready (PawnClientRestart).
	 */
	void SetupBindings(UEnhancedInputComponent* InputComponent, const TArray<FAAAAbilityInputBinding>& Bindings);

	/** Clears all registered input bindings. */
	void ClearBindings(UEnhancedInputComponent* InputComponent);

	/** Sets the ASC this component will send activation requests to. */
	void SetAbilitySystemComponent(UAbilitySystemComponent* InASC);

protected:
	/** Cached ASC pointer. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Stored bindings for reference and cleanup. */
	UPROPERTY(Transient)
	TArray<FAAAAbilityInputBinding> ActiveBindings;

	/** Called when an input action's Started trigger fires. */
	void OnAbilityInputStarted(const FGameplayTag InputTag);

	/** Called when an input action's Completed trigger fires. */
	void OnAbilityInputCompleted(const FGameplayTag InputTag, bool bCancelOnRelease);
};
