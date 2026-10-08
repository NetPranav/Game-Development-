// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRRobotVisualComponent.h"
#include "RRGame.h"
#include "RRSettings.h"
#include "AAACombatComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URRRobotVisualComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_DedicatedServer)
		{
			return; // presentation only
		}
		FTimerManager& TM = World->GetTimerManager();
		TM.SetTimer(ApplyTimerA, this, &URRRobotVisualComponent::Apply, 0.3f, false);
		TM.SetTimer(ApplyTimerB, this, &URRRobotVisualComponent::Apply, 1.5f, false);
	}
}

void URRRobotVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ApplyTimerA);
		World->GetTimerManager().ClearTimer(ApplyTimerB);
	}
	Super::EndPlay(EndPlayReason);
}

void URRRobotVisualComponent::Apply()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TSubclassOf<AActor> ClassToUse = VisualClass;
	if (!ClassToUse)
	{
		if (const URRSettings* Settings = URRSettings::Get())
		{
			ClassToUse = Settings->PlayerVisualClass.LoadSynchronous();
		}
	}
	if (!ClassToUse)
	{
		UE_LOG(LogRR, Warning, TEXT("URRRobotVisualComponent: no visual class configured"));
		return;
	}

	UChildActorComponent* VisualOverride = nullptr;
	USkeletalMeshComponent* MasterMesh = nullptr;
	TInlineComponentArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Comp : Components)
	{
		if (UChildActorComponent* Child = Cast<UChildActorComponent>(Comp))
		{
			if (Child->GetName().Contains(TEXT("VisualOverride")))
			{
				VisualOverride = Child;
			}
		}
		else if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Comp))
		{
			// The master mannequin is the one that is not part of a child actor.
			if (Mesh->GetOwner() == Owner && !MasterMesh)
			{
				MasterMesh = Mesh;
			}
		}
	}

	if (!VisualOverride || !MasterMesh)
	{
		UE_LOG(LogRR, Warning, TEXT("URRRobotVisualComponent: VisualOverride child actor (%d) or master mesh (%d) not found on %s"),
			VisualOverride ? 1 : 0, MasterMesh ? 1 : 0, *GetNameSafe(Owner));
		return;
	}

	if (VisualOverride->GetChildActorClass() != ClassToUse)
	{
		VisualOverride->SetChildActorClass(ClassToUse);
	}

	// The display actor reads the master's pose, so keep evaluating it even though it is not drawn.
	MasterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MasterMesh->SetHiddenInGame(true, /*bPropagateToChildren*/ false);

	if (UAAACombatComponent* Combat = Owner->FindComponentByClass<UAAACombatComponent>())
	{
		Combat->RefreshWeaponAttachment();
	}

	UE_LOG(LogRR, Log, TEXT("Player visual set to %s (master mesh %s hidden)"), *GetNameSafe(ClassToUse), *GetNameSafe(MasterMesh));
}
