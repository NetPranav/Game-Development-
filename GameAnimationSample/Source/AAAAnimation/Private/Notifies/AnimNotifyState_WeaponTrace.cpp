// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Notifies/AnimNotifyState_WeaponTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "AAACombatComponent.h"
#include "AAAAWeaponActor.h"
#include "GameFramework/Character.h"

void UAnimNotifyState_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UAAACombatComponent* CombatComp = MeshComp->GetOwner()->FindComponentByClass<UAAACombatComponent>())
		{
			if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
			{
				Weapon->StartWeaponTrace();
			}
		}
	}
}

void UAnimNotifyState_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UAAACombatComponent* CombatComp = MeshComp->GetOwner()->FindComponentByClass<UAAACombatComponent>())
		{
			if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
			{
				Weapon->StopWeaponTrace();
			}
		}
	}
}
