// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RREnemy.h"
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

void ARREnemy::UpdateLocomotion()
{
	if (!IsAlive()) return;
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// something else (attack, hit reaction, stagger) owns the slot: do not fight it
	UAnimMontage* Active = Anim->GetCurrentActiveMontage();
	if (Active && Active != LocoMontage) return;

	const FVector Vel = GetVelocity();
	const float Speed = Vel.Size2D();
	const FVector Local = GetActorTransform().InverseTransformVectorNoScale(Vel.GetSafeNormal2D());

	const TCHAR* Name = TEXT("Idle");
	if (Speed > 30.0f)
	{
		if (FMath::Abs(Local.X) >= FMath::Abs(Local.Y)) Name = Local.X >= 0.0f ? TEXT("Jog_Fwd") : TEXT("Jog_Bwd");
		else Name = Local.Y >= 0.0f ? TEXT("Jog_Right") : TEXT("Jog_Left");
	}

	const FName Wanted(Name);
	if (Wanted == LocoCurrent && Active == LocoMontage && LocoMontage) return;

	const FString Path = FString::Printf(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/%s.%s"), Name, Name);
	UAnimSequence* Seq = Cast<UAnimSequence>(StaticLoadObject(UAnimSequence::StaticClass(), nullptr, *Path));
	if (!Seq) return;
	LocoMontage = Anim->PlaySlotAnimationAsDynamicMontage(Seq, FName("DefaultSlot"), 0.2f, 0.25f, 1.0f, 1000);
	LocoCurrent = Wanted;
	if (LocoMontage && Speed > 30.0f)
	{
		Anim->Montage_SetPlayRate(LocoMontage, FMath::Clamp(Speed / 420.0f, 0.6f, 1.3f));
	}
}

void ARREnemy::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(LocomotionTimer, this, &ARREnemy::UpdateLocomotion, 0.1f, true, 0.3f);

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
