// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AAAAPlayerPawn.h"
#include "AAAATestEnemy.generated.h"

class USkeletalMeshComponent;
class UAAAAbilitySystemComponent;
class UAAAAttributeSet;

/**
 * A basic test enemy for the Phase 8 Combat Vertical Slice.
 * Simply stands still, receives damage, and dies when health reaches 0.
 */
UCLASS()
class AAAAI_API AAAAATestEnemy : public AAAAAPlayerPawn
{
	GENERATED_BODY()
	
public:
	AAAAATestEnemy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	/** Overrides the base class OnDeath to add ragdoll and destruction logic */
	virtual void OnDeath() override;

protected:
	/** Optional skeletal mesh component if not using the base class mesh (APawn has no default mesh) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Abilities")
	TObjectPtr<UAAAAbilitySystemComponent> HardRefASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA|Abilities")
	TObjectPtr<UAAAAttributeSet> HardRefAttributeSet;
};
