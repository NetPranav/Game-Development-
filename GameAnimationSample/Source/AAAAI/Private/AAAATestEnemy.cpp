// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAATestEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AAAAbilitySystemComponent.h"
#include "AAAAttributeSet.h"
#include "AAAAttributeComponent.h"

AAAAATestEnemy::AAAAATestEnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Since APawn doesn't have a Capsule or Mesh by default, we can add them here for the enemy if needed.
	// We'll create a basic capsule and mesh for testing.
	
	UCapsuleComponent* CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CapsuleComp;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	MeshComponent->SetCollisionProfileName(TEXT("CharacterMesh"));

	// Create and hold hard references to Ability System components
	HardRefASC = CreateDefaultSubobject<UAAAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HardRefASC->SetIsReplicated(true);
	HardRefASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HardRefAttributeSet = CreateDefaultSubobject<UAAAAttributeSet>(TEXT("AttributeSet"));

	// Set base class weak pointers
	AbilitySystemComponent = HardRefASC;
	AttributeSet = HardRefAttributeSet;
}

void AAAAATestEnemy::BeginPlay()
{
	Super::BeginPlay();

	// In a real scenario, enemies might be possessed by an AIController, which calls InitAbilitySystem.
	// For this test dummy, we'll force init it here so it has stats immediately.
	if (HasAuthority())
	{
		HardRefASC->InitAbilityActorInfo(this, this);
		if (AttributeComponent)
		{
			AttributeComponent->InitializeWithAbilitySystem(HardRefASC);
		}
	}
}

void AAAAATestEnemy::OnDeath()
{
	Super::OnDeath();

	// Simple Ragdoll
	if (MeshComponent)
	{
		MeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComponent->SetAllBodiesSimulatePhysics(true);
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->WakeAllRigidBodies();
		MeshComponent->bBlendPhysics = true;
	}

	// Disable main collision
	if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(RootComponent))
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Destroy after 5 seconds
	SetLifeSpan(5.0f);
}
