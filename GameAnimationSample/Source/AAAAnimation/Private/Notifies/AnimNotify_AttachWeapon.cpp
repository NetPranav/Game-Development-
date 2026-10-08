// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Notifies/AnimNotify_AttachWeapon.h"
#include "AAACombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotify_AttachWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (UAAACombatComponent* CombatComp = OwnerActor->FindComponentByClass<UAAACombatComponent>())
	{
		if (TargetSocket == EAAAWeaponAttachmentTarget::Hand)
		{
			CombatComp->AttachWeaponToHand();
		}
		else
		{
			CombatComp->AttachWeaponToHolster();
		}
	}
}
