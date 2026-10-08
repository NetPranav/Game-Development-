// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
struct FHitResult;

DECLARE_MULTICAST_DELEGATE_FiveParams(FAAAOnHitConfirmedNative, AActor* /*Attacker*/, AActor* /*Victim*/, const FHitResult& /*Hit*/, bool /*bHeavy*/, float /*Damage*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FAAAOnBlockedNative, AActor* /*Attacker*/, AActor* /*Victim*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FAAAOnParriedNative, AActor* /*Attacker*/, AActor* /*Victim*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FAAAOnStaggeredNative, AActor* /*Victim*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FAAAOnDeathNative, AActor* /*Victim*/);

struct AAACOMBAT_API FAAACombatEvents
{
public:
	static FAAAOnHitConfirmedNative& OnHitConfirmed();
	static FAAAOnBlockedNative& OnBlocked();
	static FAAAOnParriedNative& OnParried();
	static FAAAOnStaggeredNative& OnStaggered();
	static FAAAOnDeathNative& OnDeath();
};
