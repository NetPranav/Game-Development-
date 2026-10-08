// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AttachWeapon.generated.h"

UENUM(BlueprintType)
enum class EAAAWeaponAttachmentTarget : uint8
{
	Hand		UMETA(DisplayName = "Equipped in Hand"),
	Holster		UMETA(DisplayName = "Holstered on Back / Hip")
};

/**
 * AnimNotify that snaps the equipped weapon to the hand or holster socket
 * at the exact frame of contact during Draw / Sheath montages.
 */
UCLASS(meta = (DisplayName = "AAA Attach Weapon"))
class AAAANIMATION_API UAnimNotify_AttachWeapon : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** Where to attach the weapon when this notify triggers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	EAAAWeaponAttachmentTarget TargetSocket = EAAAWeaponAttachmentTarget::Hand;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
