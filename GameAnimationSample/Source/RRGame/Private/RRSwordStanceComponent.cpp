// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRSwordStanceComponent.h"
#include "RRGame.h"
#include "AAACombatComponent.h"
#include "AAAGameplayTags.h"
#include "AAAAttributeComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarRRStance(TEXT("rr.stance"), 1, TEXT("1 = sword-and-shield locomotion while the sword is drawn"));

namespace
{
	// Pro Sword & Shield loops (in place after Tools/editor_scripts/pss_repair.py) and the speed each was authored at (cm/s).
	constexpr float WalkSpeed = 133.0f, RunSpeed = 377.0f, BackWalkSpeed = 99.0f, BackRunSpeed = 402.0f;
	constexpr float StrafeWalkSpeed = 113.0f, StrafeRunSpeed = 315.0f;
}

URRSwordStanceComponent::URRSwordStanceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void URRSwordStanceComponent::BeginPlay()
{
	Super::BeginPlay();
	for (const TCHAR* N : { TEXT("Idle"), TEXT("BlockIdle"), TEXT("Walk"), TEXT("Walk2"), TEXT("Run"), TEXT("Run2"),
		TEXT("Strafe"), TEXT("Strafe2"), TEXT("Strafe3"), TEXT("Strafe4") })
	{
		Load(N);   // warm the cache so the first step never hitches
	}
}

UAnimSequence* URRSwordStanceComponent::Load(const TCHAR* Asset)
{
	const FName Key(Asset);
	if (TObjectPtr<UAnimSequence>* Found = Cache.Find(Key)) return Found->Get();
	UAnimSequence* Seq = LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("/Game/RR/Player/Anims/PSS/RT_PSS_%s.RT_PSS_%s"), Asset, Asset));
	if (!Seq) UE_LOG(LogRR, Warning, TEXT("Sword stance clip missing: %s"), Asset);
	Cache.Add(Key, Seq);
	return Seq;
}

void URRSwordStanceComponent::PlayLoop(UAnimInstance* Anim, FName Key, const TCHAR* Asset, float Rate)
{
	if (Key == Current && LoopMontage && Anim->Montage_IsPlaying(LoopMontage))
	{
		Anim->Montage_SetPlayRate(LoopMontage, Rate);
		return;
	}
	UAnimSequence* Seq = Load(Asset);
	if (!Seq) return;
	LoopMontage = Anim->PlaySlotAnimationAsDynamicMontage(Seq, Slot, 0.22f, 0.22f, Rate, 100000);
	Current = Key;
}

void URRSwordStanceComponent::StopLoop(UAnimInstance* Anim, float Blend)
{
	if (Anim && LoopMontage && Anim->Montage_IsPlaying(LoopMontage))
	{
		Anim->Montage_Stop(Blend, LoopMontage);
	}
	LoopMontage = nullptr;
	Current = NAME_None;
}

void URRSwordStanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* Pawn = Cast<APawn>(GetOwner());
	UAAACombatComponent* Combat = Pawn ? Pawn->FindComponentByClass<UAAACombatComponent>() : nullptr;
	UAnimInstance* Anim = Combat ? Combat->FindAnimInstance() : nullptr;
	if (!Anim) return;

	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
	const bool bDead = Combat->HasState(Tags.State_Combat_Dead);
	const bool bWant = CVarRRStance.GetValueOnGameThread() != 0 && Combat->IsWeaponDrawn() && !bDead;
	if (!bWant)
	{
		if (LoopMontage) StopLoop(Anim, 0.3f);
		return;
	}

	// an action montage owns the body: let it play, resume afterwards
	UAnimMontage* Active = Anim->GetCurrentActiveMontage();
	if (Active && Active != LoopMontage)
	{
		LoopMontage = nullptr;
		Current = NAME_None;
		return;
	}

	FVector Vel = Pawn->GetVelocity();
	Vel.Z = 0.0f;
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Vel.Size(), DeltaTime, 10.0f);
	const float Speed = SmoothedSpeed;

	if (Combat->HasState(Tags.State_Combat_Blocking) && Speed < 60.0f)
	{
		PlayLoop(Anim, TEXT("Guard"), TEXT("BlockIdle"), 1.0f);
		return;
	}
	if (Speed < 25.0f)
	{
		PlayLoop(Anim, TEXT("Idle"), TEXT("Idle"), 1.0f);
		return;
	}

	// direction relative to where the body faces
	const FVector Local = Pawn->GetActorTransform().InverseTransformVectorNoScale(Vel.GetSafeNormal());
	const bool bRun = Speed > 230.0f;
	if (Local.X > 0.6f)
	{
		const float Ref = bRun ? RunSpeed : WalkSpeed;
		PlayLoop(Anim, bRun ? TEXT("Run") : TEXT("Walk"), bRun ? TEXT("Run") : TEXT("Walk"), FMath::Clamp(Speed / Ref, 0.6f, 1.5f));
	}
	else if (Local.X < -0.6f)
	{
		const float Ref = bRun ? BackRunSpeed : BackWalkSpeed;
		PlayLoop(Anim, bRun ? TEXT("BackRun") : TEXT("BackWalk"), bRun ? TEXT("Run2") : TEXT("Walk2"), FMath::Clamp(Speed / Ref, 0.6f, 1.5f));
	}
	else
	{
		// actor +Y is right. Strafe/Strafe4 travel right, Strafe2/Strafe3 travel left (measured from the source hips)
		const bool bRight = Local.Y > 0.0f;
		const float Ref = bRun ? StrafeRunSpeed : StrafeWalkSpeed;
		const TCHAR* Clip = bRight ? (bRun ? TEXT("Strafe4") : TEXT("Strafe")) : (bRun ? TEXT("Strafe3") : TEXT("Strafe2"));
		PlayLoop(Anim, FName(Clip), Clip, FMath::Clamp(Speed / Ref, 0.6f, 1.5f));
	}
}
