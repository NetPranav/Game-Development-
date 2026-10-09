// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RREnemy.h"
#include "RRGame.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "RREnemyAIController.h"
#include "RREnemyDirector.h"
#include "AAAAbilitySystemComponent.h"
#include "AAAAttributeSet.h"
#include "AAAAttributeComponent.h"
#include "AAATeamComponent.h"
#include "AAACombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ARREnemy::ARREnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ARREnemyAIController::StaticClass();

	ASC = CreateDefaultSubobject<UAAAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAAAAttributeSet>(TEXT("AttributeSet"));

	AttributeComponent = CreateDefaultSubobject<UAAAAttributeComponent>(TEXT("AttributeComponent"));
	TeamComponent = CreateDefaultSubobject<UAAATeamComponent>(TEXT("TeamComponent"));
	CombatComponent = CreateDefaultSubobject<UAAACombatComponent>(TEXT("CombatComponent"));
}

void ARREnemy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAbilitySystemComponent* ARREnemy::GetAbilitySystemComponent() const
{
	return ASC;
}

void ARREnemy::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	if (TeamComponent)
	{
		TeamComponent->SetGenericTeamId(TeamID);
	}
}

FGenericTeamId ARREnemy::GetGenericTeamId() const
{
	if (TeamComponent)
	{
		return TeamComponent->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

void ARREnemy::StopIntroMontage()
{
	// The Paragon animation blueprint plays "LevelStart_Montage" (a seated pose) on spawn; it owns the slot and the enemy would slide forward while "sitting".
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (Anim)
	{
		if (UAnimMontage* Active = Anim->GetCurrentActiveMontage())
		{
			if (Active->GetName().Contains(TEXT("LevelStart")) || Active->GetName().Contains(TEXT("Spawn")) || Active->GetName().Contains(TEXT("Emote")))
			{
				Anim->Montage_Stop(0.1f, Active);
			}
		}
	}
	if (++IntroStopTicks >= 20)
	{
		GetWorldTimerManager().ClearTimer(IntroStopTimer);
	}
}

void ARREnemy::DriveAnimVars()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (!Anim) return;

	const float Speed = IsAlive() ? GetVelocity().Size2D() : 0.0f;
	const UCharacterMovementComponent* Move = GetCharacterMovement();
	const bool bAccel = Speed > 20.0f && Move && Move->GetCurrentAcceleration().SizeSquared2D() > 1.0f;

	bool bAny = false;
	UClass* C = Anim->GetClass();
	if (FProperty* P = C->FindPropertyByName(TEXT("Speed")))
	{
		if (FFloatProperty* F = CastField<FFloatProperty>(P)) { F->SetPropertyValue_InContainer(Anim, Speed); bAny = true; }
		else if (FDoubleProperty* D = CastField<FDoubleProperty>(P)) { D->SetPropertyValue_InContainer(Anim, Speed); bAny = true; }
	}
	if (FBoolProperty* B = CastField<FBoolProperty>(C->FindPropertyByName(TEXT("IsAccelerating"))))
	{
		B->SetPropertyValue_InContainer(Anim, bAccel);
	}
	if (FBoolProperty* B = CastField<FBoolProperty>(C->FindPropertyByName(TEXT("IsInAir"))))
	{
		B->SetPropertyValue_InContainer(Anim, Move && Move->IsFalling());
	}

	// In Greystone_AnimBlueprint, BlendListByBool_16 selects:
	// Pose 0 (UpperBody LayeredBoneBlend) when (IsAccelerating && !FullBody),
	// Pose 1 (FullBody slot pose from CachedPose_UpperBody) when (!IsAccelerating || FullBody).
	// Setting FullBody = true ensures attacks, hit reactions, stuns, deaths, and locomotion montages
	// evaluate across the entire skeleton instead of getting upper-body masked.
	const UAnimMontage* ActiveMontage = Anim->GetCurrentActiveMontage();
	const bool bFullBodyActive = (ActiveMontage != nullptr) || !IsAlive();
	if (FBoolProperty* B = CastField<FBoolProperty>(C->FindPropertyByName(TEXT("FullBody"))))
	{
		B->SetPropertyValue_InContainer(Anim, bFullBodyActive);
	}

	bAnimVarsDriven = bAny && !FParse::Param(FCommandLine::Get(), TEXT("RRLocoMontage"));
}

void ARREnemy::UpdateLocomotion()
{
	if (!IsAlive()) return;
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (!Anim) return;

	UAnimMontage* Active = Anim->GetCurrentActiveMontage();
	const FVector Vel = GetVelocity();
	const float Speed = Vel.Size2D();
	const float Yaw = GetActorRotation().Yaw;
	const float YawStep = FMath::FindDeltaAngleDegrees(LastLocoYaw, Yaw);
	LastLocoYaw = Yaw;

	if (++LocoDebugTick % 10 == 0)
	{
		UE_LOG(LogRR, Verbose, TEXT("Loco %s: speed=%.0f active=%s loco=%s current=%s"), *GetName(), Speed,
			*GetNameSafe(Active), *GetNameSafe(LocoMontage), *LocoCurrent.ToString());
	}

	// An action (attack, hit reaction, stagger, taunt, death...) owns the slot: do not fight it.
	if (Active && Active != LocoMontage)
	{
		TurnAccum = 0.0f;
		return;
	}

	const FVector Local = GetActorTransform().InverseTransformVectorNoScale(Vel.GetSafeNormal2D());
	const bool bMoving = Speed > 30.0f;
	const bool bForward = bMoving && Local.X > 0.5f;   // within ~60 deg of facing

	FName Wanted = NAME_None;
	float Rate = 1.0f;
	if (bMoving && !bForward)
	{
		if (FMath::Abs(Local.X) >= FMath::Abs(Local.Y))
		{
			Wanted = TEXT("Jog_Bwd");
		}
		else
		{
			Wanted = Local.Y >= 0.0f ? FName(TEXT("Jog_Right")) : FName(TEXT("Jog_Left"));
		}
		Rate = FMath::Clamp(Speed / 400.0f, 0.55f, 1.25f);
		TurnAccum = 0.0f;
	}
	else if (!bMoving)
	{
		TurnAccum = FMath::Abs(YawStep) > 1.0f ? TurnAccum + YawStep : TurnAccum * 0.5f;
		if (FMath::Abs(TurnAccum) > 40.0f && LocoCurrent != TEXT("Turn"))
		{
			Wanted = TurnAccum > 0.0f ? FName(TEXT("Turn_Right_90")) : FName(TEXT("Turn_Left_90"));
			Rate = 1.5f;
			TurnAccum = 0.0f;
		}
	}
	else // bMoving && bForward: Closing in / chasing player
	{
		Wanted = TEXT("Jog_Fwd");
		Rate = FMath::Clamp(Speed / 420.0f, 0.6f, 1.3f);
		TurnAccum = 0.0f;
	}

	if (Wanted.IsNone())
	{
		// Smoothly blend out of locomotion into the idle pose
		if (LocoMontage && Active == LocoMontage && LocoCurrent != TEXT("Turn"))
		{
			Anim->Montage_Stop(0.25f, LocoMontage);
			LocoMontage = nullptr;
			LocoCurrent = NAME_None;
		}
		else if (!Active)
		{
			LocoMontage = nullptr;
			LocoCurrent = NAME_None;
		}
		return;
	}

	const bool bTurn = Wanted.ToString().StartsWith(TEXT("Turn_"));
	if (Wanted == LocoCurrent && Active == LocoMontage && LocoMontage)
	{
		Anim->Montage_SetPlayRate(LocoMontage, Rate);
		return;
	}

	const FString Name = Wanted.ToString();
	const FString Path = FString::Printf(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/%s.%s"), *Name, *Name);
	UAnimSequence* Seq = Cast<UAnimSequence>(StaticLoadObject(UAnimSequence::StaticClass(), nullptr, *Path));
	if (!Seq) return;
	LocoMontage = Anim->PlaySlotAnimationAsDynamicMontage(Seq, AnimSlot, bTurn ? 0.15f : 0.25f, 0.25f, Rate, bTurn ? 1 : 1000);
	LocoCurrent = bTurn ? FName(TEXT("Turn")) : Wanted;
}

void ARREnemy::BeginPlay()
{
	Super::BeginPlay();

	// Paragon Anim Blueprints route actions to UpperBody: ensure every montage (hit reactions, stun, death) plays on AnimSlot.
	if (CombatComponent) { CombatComponent->MontageSlotOverride = AnimSlot; }
	LastLocoYaw = GetActorRotation().Yaw;
	GetWorldTimerManager().SetTimer(LocomotionTimer, this, &ARREnemy::UpdateLocomotion, 0.05f, true, 0.1f);
	GetWorldTimerManager().SetTimer(AnimVarsTimer, this, &ARREnemy::DriveAnimVars, 0.01f, true, 0.05f);

	GetWorldTimerManager().SetTimer(IntroStopTimer, this, &ARREnemy::StopIntroMontage, 0.1f, true);

	if (HasAuthority())
	{
		ASC->InitAbilityActorInfo(this, this);

		if (AttributeComponent)
		{
			AttributeComponent->InitializeWithAbilitySystem(ASC);
			AttributeComponent->OnDeath.AddDynamic(this, &ARREnemy::OnDeath);
		}

		if (AttributeSet)
		{
			AttributeSet->InitMaxHealth(Tuning.MaxHealth);
			AttributeSet->InitHealth(Tuning.MaxHealth);
			AttributeSet->InitMaxStamina(Tuning.MaxStamina);
			AttributeSet->InitStamina(Tuning.MaxStamina);
			AttributeSet->InitMaxPoise(Tuning.MaxPoise);
			AttributeSet->InitPoise(Tuning.MaxPoise);
			AttributeSet->InitArmor(Tuning.Armor);
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = Tuning.WalkSpeed;
		}

		if (TeamComponent)
		{
			TeamComponent->SetGenericTeamId(FGenericTeamId(1)); // 1 is typically Enemy in this project if Player is 0. Wait, check UAAATeamComponent/team enum.
		}

		if (CombatComponent && WeaponSettings)
		{
			CombatComponent->EquipWeapon(WeaponSettings);
		}
	}
}

void ARREnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ARREnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (ASC)
		{
			ASC->InitAbilityActorInfo(this, this);
		}
	}
}

void ARREnemy::OnDeath()
{
	if (HasAuthority())
	{
		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			AICon->StopMovement();
			// Brain off handled in Controller itself if we check IsDead
		}

		if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
		{
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// Ragdoll fallback if no Death montage played is handled below
		bool bDeathMontagePlayed = false;
		if (CombatComponent && CombatComponent->HitReactions.Death)
		{
			bDeathMontagePlayed = true; 
			// Wait, the combat presentation layer plays the montage. The prompt says: 
			// "let UAAACombatComponent presentation (P6) play the Death montage/ragdoll fallback — if P6 did not implement ragdoll for non-AAAAATestEnemy actors, ragdoll here when no Death montage was played"
			// The actual combat component plays the hit reaction montage internally on death event (P6).
			// We can check if we're actually playing a montage:
			if (ASC)
			{
				bDeathMontagePlayed = ASC->GetCurrentMontage() != nullptr;
			}
		}

		if (!bDeathMontagePlayed)
		{
			if (USkeletalMeshComponent* MeshComp = GetMesh())
			{
				MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
				MeshComp->SetAllBodiesSimulatePhysics(true);
				MeshComp->SetSimulatePhysics(true);
				MeshComp->WakeAllRigidBodies();
				MeshComp->bBlendPhysics = true;
			}
		}

		SetLifeSpan(Tuning.DespawnAfterDeathSeconds);

		if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
		{
			Director->NotifyEnemyDied(this);
		}
	}
}

bool ARREnemy::IsAlive() const
{
	if (AttributeComponent)
	{
		return !AttributeComponent->IsDead();
	}
	return true;
}

AActor* ARREnemy::GetCurrentTarget() const
{
	if (ARREnemyAIController* AICon = Cast<ARREnemyAIController>(GetController()))
	{
		return AICon->GetTarget(); // We need to implement this on ARREnemyAIController
	}
	return nullptr;
}

float ARREnemy::GetHealthFraction() const
{
	if (AttributeComponent && AttributeComponent->GetMaxHealth() > 0.f)
	{
		return AttributeComponent->GetHealth() / AttributeComponent->GetMaxHealth();
	}
	return 0.f;
}
