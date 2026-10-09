// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RRSwordStanceComponent.generated.h"

class UAnimMontage;
class UAnimSequence;
class UAnimInstance;

/**
 * Sword-and-shield stance for the player while the weapon is drawn: idle / walk / run / strafe / back-pedal / guard
 * loops from the Pro Sword & Shield pack (RT_PSS_*) replace the unarmed motion-matching locomotion on the master mesh.
 * Plays full-body looping slot montages only when no action montage (attack, hit, draw...) owns the slot, so every
 * attack / reaction still wins and the stance resumes right after. Play rate follows the real speed (no foot sliding).
 * Sheathing the sword hands locomotion back to the Game Animation Sample. Console: rr.stance 0/1.
 */
UCLASS(ClassGroup = (RR), meta = (BlueprintSpawnableComponent))
class RRGAME_API URRSwordStanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URRSwordStanceComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "RR|Stance")
	FName Slot = TEXT("DefaultSlot");

protected:
	virtual void BeginPlay() override;

private:
	struct FClip { FName Key; const TCHAR* Asset; float Speed; };
	UAnimSequence* Load(const TCHAR* Asset);
	void PlayLoop(UAnimInstance* Anim, FName Key, const TCHAR* Asset, float Rate);
	void StopLoop(UAnimInstance* Anim, float Blend);

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> LoopMontage;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UAnimSequence>> Cache;

	FName Current;
	float SmoothedSpeed = 0.0f;
};
