// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Notifies/AnimNotifyState_ComboWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "AAACombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AAAGameplayTags.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UAnimNotifyState_ComboWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UAAACombatComponent* CombatComp = MeshComp->GetOwner()->FindComponentByClass<UAAACombatComponent>())
		{
			EAAAAttackType BufferedType;
			if (CombatComp->ConsumeBufferedAttack(BufferedType))
			{
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(MeshComp->GetOwner()))
				{
					if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
					{
						FGameplayTagContainer TagContainer;
						TagContainer.AddTag(BufferedType == EAAAAttackType::Light ? FAAAGameplayTags::Get().InputTag_Attack : FAAAGameplayTags::Get().InputTag_AttackHeavy);
						ASC->TryActivateAbilitiesByTag(TagContainer);
					}
				}
			}
		}
	}
}
