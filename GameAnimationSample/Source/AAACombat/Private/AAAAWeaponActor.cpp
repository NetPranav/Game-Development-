// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAWeaponActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAssets/AAAWeaponSettings.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "AAAGameplayTags.h"

AAAAWeaponActor::AAAAWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create a dummy root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(Root);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // We use custom sweep tracing instead

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(Root);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsTracing = false;
}

void AAAAWeaponActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAAAWeaponActor, WeaponSettings);
}

void AAAAWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAAAWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTracing)
	{
		PerformTrace();
	}
}

void AAAAWeaponActor::InitializeWeapon(UAAAWeaponSettings* InWeaponSettings)
{
	if (!InWeaponSettings)
	{
		return;
	}

	WeaponSettings = InWeaponSettings;

	if (WeaponSettings->Visuals.StaticMesh)
	{
		StaticMeshComponent->SetStaticMesh(WeaponSettings->Visuals.StaticMesh);

		// Imported weapons rarely have their pivot at the grip. If the mesh has a "Grip" socket, shift the mesh so that
		// the socket sits at this actor's origin (= the hand socket it gets attached to).
		FVector GripOffset = FVector::ZeroVector;
		FRotator MeshExtraRot = FRotator::ZeroRotator;
		if (const UStaticMeshSocket* Grip = WeaponSettings->Visuals.StaticMesh->FindSocket(FName("Grip")))
		{
			FTransform GripTransform(Grip->RelativeRotation, Grip->RelativeLocation, FVector::OneVector);
			FTransform InvGrip = GripTransform.Inverse();
			StaticMeshComponent->SetRelativeLocationAndRotation(InvGrip.GetLocation(), InvGrip.Rotator());
		}
		else
		{
			// Dynamic fallback: detect weapon orientation from bounding box
			FBoxSphereBounds Bounds = WeaponSettings->Visuals.StaticMesh->GetBounds();
			if (Bounds.BoxExtent.X > Bounds.BoxExtent.Z && Bounds.BoxExtent.X > Bounds.BoxExtent.Y)
			{
				// Sword running along X axis (e.g. SM_LongSword): grip around X = -35, blade extends to +X.
				// Pitch -90 rotates +X to +Z (blade upwards out of hand).
				GripOffset = FVector(35.0f, 0.0f, 0.0f);
				MeshExtraRot = FRotator(-90.0f, 0.0f, 0.0f);
			}
			else if (Bounds.BoxExtent.Z > Bounds.BoxExtent.X && Bounds.BoxExtent.Z > Bounds.BoxExtent.Y)
			{
				GripOffset = FVector(0.0f, 0.0f, -130.0f);
				MeshExtraRot = FRotator(180.0f, 0.0f, 0.0f);
			}
			StaticMeshComponent->SetRelativeLocationAndRotation(GripOffset, MeshExtraRot);
		}
		StaticMeshComponent->SetVisibility(true);
		SkeletalMeshComponent->SetVisibility(false);
	}
	else if (WeaponSettings->Visuals.SkeletalMesh)
	{
		SkeletalMeshComponent->SetSkeletalMesh(WeaponSettings->Visuals.SkeletalMesh);
		SkeletalMeshComponent->SetVisibility(true);
		StaticMeshComponent->SetVisibility(false);
	}
}

bool AAAAWeaponActor::GetBladeLine(FVector& OutStart, FVector& OutEnd) const
{
	const UStaticMesh* SM = StaticMeshComponent->GetStaticMesh();
	USceneComponent* ActiveMesh = SM ? Cast<USceneComponent>(StaticMeshComponent) : Cast<USceneComponent>(SkeletalMeshComponent);
	if (!ActiveMesh) return false;
	if (ActiveMesh->DoesSocketExist(TraceStartSocket) && ActiveMesh->DoesSocketExist(TraceEndSocket))
	{
		OutStart = ActiveMesh->GetSocketLocation(TraceStartSocket);
		OutEnd = ActiveMesh->GetSocketLocation(TraceEndSocket);
		return true;
	}
	if (!SM) return false;
	// No sockets: the blade runs along the mesh's longest LOCAL axis, whatever rotation the grip correction applied.
	const FBoxSphereBounds B = SM->GetBounds();
	const FVector E = B.BoxExtent;
	FVector Axis = FVector::XAxisVector; float Len = E.X;
	if (E.Y > Len) { Axis = FVector::YAxisVector; Len = E.Y; }
	if (E.Z > Len) { Axis = FVector::ZAxisVector; Len = E.Z; }
	const FTransform& T = ActiveMesh->GetComponentTransform();
	OutStart = T.TransformPosition(B.Origin - Axis * Len * 0.6f);
	OutEnd = T.TransformPosition(B.Origin + Axis * Len);
	// the hilt is the end nearer to the owner's hand: make Start the hilt
	if (const AActor* O = GetOwner())
	{
		if (FVector::DistSquared(OutEnd, O->GetActorLocation()) < FVector::DistSquared(OutStart, O->GetActorLocation()))
		{
			Swap(OutStart, OutEnd);
		}
	}
	return true;
}

void AAAAWeaponActor::StartWeaponTrace()
{
	bIsTracing = true;
	HitActors.Empty();
	PreviousBladePositions.Empty();
	
	FVector StartPos, EndPos;
	const bool bLine = GetBladeLine(StartPos, EndPos);
	UE_LOG(LogTemp, Verbose, TEXT("[TRACE] %s start (owner %s) line=%d %s -> %s mesh=%s"), *GetName(), *GetNameSafe(GetOwner()), (int32)bLine,
		*StartPos.ToCompactString(), *EndPos.ToCompactString(), *GetNameSafe(StaticMeshComponent->GetStaticMesh()));
	if (bLine)
	{
		for (int32 i = 0; i < NumBladeSamples; ++i)
		{
			float Alpha = (NumBladeSamples > 1) ? ((float)i / (NumBladeSamples - 1)) : 1.0f;
			PreviousBladePositions.Add(FMath::Lerp(StartPos, EndPos, Alpha));
		}
	}
}

void AAAAWeaponActor::StopWeaponTrace()
{
	bIsTracing = false;
	HitActors.Empty();
	PreviousBladePositions.Empty();
}

void AAAAWeaponActor::ProcessHit(const FHitResult& Hit, AActor* MyOwner, IGenericTeamAgentInterface* MyTeamAgent, TArray<AActor*>& ActorsToIgnore)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActors.Contains(HitActor))
	{
		return;
	}

	// Team check: never hit teammates.
	if (MyTeamAgent)
	{
		if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(HitActor))
		{
			if (MyTeamAgent->GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId())
			{
				return;
			}
		}
	}

	// Only actors with an ability system are damageable (floors, props and walls must not register as hits).
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC || TargetASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
	{
		return;
	}

	HitActors.Add(HitActor);
	ActorsToIgnore.Add(HitActor); // Ignore in subsequent samples this frame

	FGameplayEventData Payload;
	Payload.Instigator = MyOwner;
	Payload.Target = HitActor;

	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	TargetData->HitResult = Hit;
	Payload.TargetData.Add(TargetData);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MyOwner, FAAAGameplayTags::Get().GameplayEvent_Combat_Hit, Payload);
}

void AAAAWeaponActor::PerformBodySweep()
{
	// Used when the weapon has no mesh of its own (creatures whose weapon is part of the body mesh, e.g. Paragon
	// characters): sweep a thick sphere along the owner's facing direction for the duration of the trace window.
	AActor* MyOwner = GetOwner();
	if (!MyOwner)
	{
		return;
	}

	const float Reach = WeaponSettings ? FMath::Max(WeaponSettings->Combat.Range, 100.0f) : 190.0f;
	const FVector Origin = MyOwner->GetActorLocation();
	const FVector End = Origin + MyOwner->GetActorForwardVector() * Reach;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(MyOwner);
	TArray<AActor*> AttachedActors;
	MyOwner->GetAttachedActors(AttachedActors);
	ActorsToIgnore.Append(AttachedActors);
	for (AActor* AlreadyHit : HitActors)
	{
		if (AlreadyHit)
		{
			ActorsToIgnore.Add(AlreadyHit);
		}
	}

	TArray<FHitResult> HitResults;
	if (UKismetSystemLibrary::SphereTraceMulti(this, Origin, End, BodySweepRadius, UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true))
	{
		IGenericTeamAgentInterface* MyTeamAgent = Cast<IGenericTeamAgentInterface>(MyOwner);
		for (const FHitResult& Hit : HitResults)
		{
			ProcessHit(Hit, MyOwner, MyTeamAgent, ActorsToIgnore);
		}
	}
}

void AAAAWeaponActor::PerformTrace()
{
	if (!StaticMeshComponent->GetStaticMesh() && !SkeletalMeshComponent->GetSkeletalMeshAsset())
	{
		PerformBodySweep();
		return;
	}

	FVector StartLocation, EndLocation;
	if (!GetBladeLine(StartLocation, EndLocation)) return;
	{

#if !(UE_BUILD_SHIPPING)
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime - LastFallbackWarnTime > 3.0)
		{
			LastFallbackWarnTime = CurrentTime;
			FString WarnMsg = FString::Printf(TEXT("[COMBAT WARNING] Weapon '%s' mesh is missing '%s' or '%s' sockets! Using Bounding Box fallback."),
				*GetNameSafe(this), *TraceStartSocket.ToString(), *TraceEndSocket.ToString());
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Red, WarnMsg);
			}
			UE_LOG(LogTemp, Warning, TEXT("%s"), *WarnMsg);
		}
#endif
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		ActorsToIgnore.Add(MyOwner);
		TArray<AActor*> AttachedActors;
		MyOwner->GetAttachedActors(AttachedActors);
		ActorsToIgnore.Append(AttachedActors);
	}

	for (AActor* HitActor : HitActors)
	{
		if (HitActor)
		{
			ActorsToIgnore.Add(HitActor);
		}
	}

	IGenericTeamAgentInterface* MyTeamAgent = Cast<IGenericTeamAgentInterface>(MyOwner);

	for (int32 i = 0; i < NumBladeSamples; ++i)
	{
		if (!PreviousBladePositions.IsValidIndex(i))
		{
			continue;
		}

		float Alpha = (NumBladeSamples > 1) ? ((float)i / (NumBladeSamples - 1)) : 1.0f;
		FVector CurrentSampleLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
		FVector PrevSampleLocation = PreviousBladePositions[i];

		TArray<FHitResult> HitResults;
		bool bHit = UKismetSystemLibrary::SphereTraceMulti(
			this,
			PrevSampleLocation,
			CurrentSampleLocation,
			TraceRadius,
			UEngineTypes::ConvertToTraceType(ECC_Pawn),
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResults,
			true
		);

		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				ProcessHit(Hit, MyOwner, MyTeamAgent, ActorsToIgnore);
			}
		}

		PreviousBladePositions[i] = CurrentSampleLocation;
	}

	PerformBodySweep();
}
