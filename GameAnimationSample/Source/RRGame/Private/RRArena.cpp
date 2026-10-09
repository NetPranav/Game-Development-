// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRArena.h"
#include "RRGame.h"
#include "RRGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ARRArena::ARRArena()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PlayerSpot = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerSpot"));
	PlayerSpot->SetupAttachment(Root);
	PlayerSpot->SetRelativeLocation(FVector(-380.0f, 0.0f, 100.0f));

	BossSpot = CreateDefaultSubobject<USceneComponent>(TEXT("BossSpot"));
	BossSpot->SetupAttachment(Root);
	BossSpot->SetRelativeLocation(FVector(380.0f, 0.0f, 100.0f));

	EntryPoint = CreateDefaultSubobject<USceneComponent>(TEXT("EntryPoint"));
	EntryPoint->SetupAttachment(Root);
	EntryPoint->SetRelativeLocation(FVector(-1150.0f, 0.0f, 0.0f));

	ExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ExitPoint"));
	ExitPoint->SetupAttachment(Root);
	ExitPoint->SetRelativeLocation(FVector(-1400.0f, 250.0f, 100.0f));

	BossEntrance = CreateDefaultSubobject<USceneComponent>(TEXT("BossEntrance"));
	BossEntrance->SetupAttachment(Root);
	BossEntrance->SetRelativeLocation(FVector(1800.0f, 0.0f, 100.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	EntryDisc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntryDisc"));
	EntryDisc->SetupAttachment(EntryPoint);
	EntryDisc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EntryDisc->SetCastShadow(false);
	if (PlaneMesh.Succeeded()) EntryDisc->SetStaticMesh(PlaneMesh.Object);
	EntryDisc->SetRelativeLocation(FVector(0.0f, 0.0f, 3.0f));

	EntryGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntryGlow"));
	EntryGlow->SetupAttachment(EntryPoint);
	EntryGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EntryGlow->SetCastShadow(false);
	if (CylMesh.Succeeded()) EntryGlow->SetStaticMesh(CylMesh.Object);
}

bool ARRArena::IsInside(const FVector& Location, float Margin) const
{
	return FVector::Dist2D(Location, GetActorLocation()) < Radius + Margin;
}

static bool Ar_Ground(UWorld* W, const FVector& At, float Up, float Down, FVector& Out, const AActor* Ignore)
{
	FHitResult H;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(RRArenaGround), false, Ignore);
	if (W->LineTraceSingleByChannel(H, At + FVector(0, 0, Up), At - FVector(0, 0, Down), ECC_Visibility, Q))
	{
		Out = H.ImpactPoint;
		return true;
	}
	return false;
}

void ARRArena::SnapToGround()
{
	UWorld* W = GetWorld();
	if (!W) return;

	// Check if a Colosseum exists in the level
	AActor* ColActor = nullptr;
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(W, AActor::StaticClass(), AllActors);
	for (AActor* A : AllActors)
	{
		if (A && (A->GetName().Contains(TEXT("Colosseum")) || A->GetActorLabel().Contains(TEXT("Colosseum"))))
		{
			ColActor = A;
			break;
		}
	}

	FVector G;
	if (ColActor)
	{
		FVector ColCenter = ColActor->GetActorLocation();
		FRotator ColRot = ColActor->GetActorRotation();
		SetActorRotation(ColRot);

		// Trace floor at Colosseum center
		if (Ar_Ground(W, ColCenter + FVector(0.0f, 0.0f, 1500.0f), 2000.0f, 6000.0f, G, this))
		{
			SetActorLocation(G);
		}
		else
		{
			SetActorLocation(ColCenter);
		}

		Radius = 1750.0f; // Mid-part green combat circle floor

		// Fighting spots inside the Colosseum on the green ground
		if (PlayerSpot) PlayerSpot->SetRelativeLocation(FVector(-450.0f, -200.0f, 50.0f));
		if (BossSpot) BossSpot->SetRelativeLocation(FVector(450.0f, 200.0f, 50.0f));
		if (BossEntrance) BossEntrance->SetRelativeLocation(FVector(850.0f, 400.0f, 50.0f));

		for (USceneComponent* C : { PlayerSpot.Get(), BossSpot.Get(), BossEntrance.Get() })
		{
			if (C && Ar_Ground(W, C->GetComponentLocation(), 1500.0f, 3000.0f, G, this))
			{
				C->SetWorldLocation(G + FVector(0.0f, 0.0f, 50.0f));
			}
		}

		// Field effect entry circle at the bottom of the stairs, right before entering the green combat floor
		FVector ColEntryWorld = ColCenter + FVector(-1630.0f, -870.0f, 200.0f);
		if (Ar_Ground(W, ColEntryWorld, 1500.0f, 4000.0f, G, this))
		{
			if (EntryPoint) EntryPoint->SetWorldLocation(G + FVector(0.0f, 0.0f, 5.0f));
		}
		else
		{
			if (EntryPoint) EntryPoint->SetWorldLocation(ColCenter + FVector(-1630.0f, -870.0f, 10.0f));
		}

		FVector ColExitWorld = ColCenter + FVector(-1900.0f, -950.0f, 200.0f);
		if (Ar_Ground(W, ColExitWorld, 1500.0f, 4000.0f, G, this))
		{
			if (ExitPoint) ExitPoint->SetWorldLocation(G + FVector(0.0f, 0.0f, 50.0f));
		}
		else
		{
			if (ExitPoint) ExitPoint->SetWorldLocation(ColCenter + FVector(-1900.0f, -950.0f, 50.0f));
		}
	}
	else
	{
		// the fighting floor
		if (Ar_Ground(W, GetActorLocation(), 2000.0f, 4000.0f, G, this)) { SetActorLocation(G); }
		// spots on the floor, entry/exit/entrance on whatever ground is under them
		for (USceneComponent* C : { PlayerSpot.Get(), BossSpot.Get(), BossEntrance.Get(), ExitPoint.Get() })
		{
			if (C && Ar_Ground(W, C->GetComponentLocation(), 2000.0f, 4000.0f, G, this)) { C->SetWorldLocation(G + FVector(0.0f, 0.0f, 50.0f)); }
		}
		if (Ar_Ground(W, EntryPoint->GetComponentLocation(), 2000.0f, 4000.0f, G, this)) { EntryPoint->SetWorldLocation(G); }
	}
}

void ARRArena::BeginPlay()
{
	Super::BeginPlay();
	SnapToGround();
	BuildWalls();

	// glow material (unlit additive, see Tools/editor_scripts/make_glow_material.py); falls back to the engine default
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/RR/Materials/M_RR_Glow.M_RR_Glow"));
	const float S = EntryRadius / 50.0f;
	EntryDisc->SetRelativeScale3D(FVector(S, S, 1.0f));
	EntryGlow->SetRelativeScale3D(FVector(S * 0.98f, S * 0.98f, 2.2f));   // soft light column, 2.2 m
	EntryGlow->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	if (Glow)
	{
		DiscMID = UMaterialInstanceDynamic::Create(Glow, this);
		DiscMID->SetScalarParameterValue(TEXT("Ring"), 1.0f);
		EntryDisc->SetMaterial(0, DiscMID);
		GlowMID = UMaterialInstanceDynamic::Create(Glow, this);
		GlowMID->SetScalarParameterValue(TEXT("Ring"), 0.0f);
		EntryGlow->SetMaterial(0, GlowMID);
	}
	else
	{
		EntryGlow->SetVisibility(false);
	}
	UpdateEntryVisual(0.0f);
	UE_LOG(LogRR, Log, TEXT("Arena at %s radius %.0f, entry at %s"), *GetActorLocation().ToCompactString(), Radius, *EntryPoint->GetComponentLocation().ToCompactString());
}

void ARRArena::BuildWalls()
{
	// a closed ring of thin boxes that only block pawns (camera, traces and projectiles pass)
	const int32 Count = FMath::Clamp(FMath::CeilToInt(2.0f * PI * Radius / 120.0f), 16, 96);
	const float Seg = 2.0f * PI * Radius / Count;
	for (int32 i = 0; i < Count; ++i)
	{
		const float A = 2.0f * PI * i / Count;
		const FVector Dir(FMath::Cos(A), FMath::Sin(A), 0.0f);
		UBoxComponent* Box = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("ArenaWall_%d"), i));
		Box->SetupAttachment(Root);
		Box->SetRelativeLocation(Dir * (Radius + 25.0f) + FVector(0.0f, 0.0f, WallHeight * 0.5f - 200.0f));
		Box->SetRelativeRotation(Dir.Rotation());
		Box->SetBoxExtent(FVector(25.0f, Seg * 0.5f + 8.0f, WallHeight * 0.5f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Box->SetCollisionObjectType(ECC_WorldStatic);
		Box->SetCollisionResponseToAllChannels(ECR_Ignore);
		Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Box->SetCanEverAffectNavigation(true);   // NPC paths go around the platform
		Box->SetHiddenInGame(true);
		Box->RegisterComponent();
		Walls.Add(Box);
	}
}

void ARRArena::SetActorPassesWalls(AActor* Actor, bool bPasses)
{
	if (!Actor) return;
	TInlineComponentArray<UPrimitiveComponent*> Prims;
	Actor->GetComponents(Prims);
	for (UPrimitiveComponent* P : Prims)
	{
		P->IgnoreActorWhenMoving(this, bPasses);
	}
}

void ARRArena::SetEntryEnabled(bool bEnabled)
{
	bEntryEnabled = bEnabled;
	EntryProgress = 0.0f;
	bTriggered = false;
	EntryDisc->SetVisibility(bEnabled);
	EntryGlow->SetVisibility(bEnabled && GlowMID != nullptr);
}

void ARRArena::UpdateEntryVisual(float RealDelta)
{
	Pulse += RealDelta;
	const float Breathe = 0.75f + 0.25f * FMath::Sin(Pulse * 2.6f);
	const float Charge = EntryProgress;   // brightens and turns white while charging
	const FLinearColor C = FMath::Lerp(EntryColor, FLinearColor(1.0f, 0.95f, 0.85f), Charge);
	if (DiscMID)
	{
		DiscMID->SetVectorParameterValue(TEXT("Color"), C);
		DiscMID->SetScalarParameterValue(TEXT("Intensity"), (6.0f + 18.0f * Charge) * Breathe);
	}
	if (GlowMID)
	{
		GlowMID->SetVectorParameterValue(TEXT("Color"), C);
		GlowMID->SetScalarParameterValue(TEXT("Intensity"), (0.35f + 2.5f * Charge) * Breathe);
	}
}

void ARRArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Real = FMath::Min((float)FApp::GetDeltaTime(), 0.1f);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	ARRGameMode* GM = GetWorld()->GetAuthGameMode<ARRGameMode>();
	const bool bCanEnter = bEntryEnabled && GM && GM->CanEnterArena();

	bPlayerNear = false;
	if (Player && bCanEnter)
	{
		const FVector P = Player->GetActorLocation();
		const FVector E = EntryPoint->GetComponentLocation();
		const float D = FVector::Dist2D(P, E);
		bPlayerNear = D < EntryRadius * 6.0f && FMath::Abs(P.Z - E.Z) < 500.0f;
		const bool bIn = D < EntryRadius && FMath::Abs(P.Z - E.Z) < 350.0f;
		EntryProgress = bIn ? FMath::Min(1.0f, EntryProgress + Real / FMath::Max(0.1f, EntryHoldSeconds))
		                    : FMath::Max(0.0f, EntryProgress - Real * 3.0f);
		if (EntryProgress >= 1.0f && !bTriggered)
		{
			bTriggered = true;
			GM->RequestArenaMatch(this);
		}
		if (!bIn) bTriggered = false;
	}
	else
	{
		EntryProgress = 0.0f;
	}

	EntryDisc->SetVisibility(bCanEnter);
	EntryGlow->SetVisibility(bCanEnter && GlowMID != nullptr);
	UpdateEntryVisual(Real);
}
