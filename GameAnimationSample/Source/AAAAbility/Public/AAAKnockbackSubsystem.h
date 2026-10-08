// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AAAKnockbackSubsystem.generated.h"

class AActor;
struct FHitResult;

/**
 * AAAKnockbackSubsystem
 * Authority-only subsystem handling physical knockback reactions.
 */
UCLASS()
class AAAABILITY_API UAAAKnockbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);
	void OnStaggered(AActor* Victim);

	FDelegateHandle HitConfirmedHandle;
	FDelegateHandle StaggeredHandle;

	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> LastAttackerMap;
};
