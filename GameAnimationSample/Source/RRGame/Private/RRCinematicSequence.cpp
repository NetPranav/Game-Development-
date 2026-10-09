// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRCinematicSequence.h"

URRCinematicSequence::URRCinematicSequence()
{
	// The player's retargeted sword animations (see Tools/editor_scripts/wire_ual2.py). Replace them in a data asset to change the scene.
	DefenderBlockAnim = FRRCineAnim(TEXT("/Game/RR/Player/AM_P_SwordBlock.AM_P_SwordBlock"));
	DefenderImpactAnim = FRRCineAnim(TEXT("/Game/RR/Player/AM_P_Knockback.AM_P_Knockback"));
	// braced sword-guard pose while being pushed back (the UAL2 "Slide" is a baseball slide on the ground, wrong silhouette)
	DefenderSlideAnim = FRRCineAnim(TEXT("/Game/RR/Player/AM_P_SwordBlock.AM_P_SwordBlock"));
	DefenderSlideAnim.PlayRate = 0.6f;
	DefenderStopAnim = FRRCineAnim(TEXT("/Game/RR/Player/AM_P_GuardBreak.AM_P_GuardBreak"));

	const TCHAR* Sparks = TEXT("/Game/ParagonGreystone/FX/Particles/Greystone/Abilities/Primary/FX/P_Greystone_Primary_Sparks.P_Greystone_Primary_Sparks");
	const TCHAR* Impact = TEXT("/Game/ParagonGreystone/FX/Particles/Greystone/Abilities/Primary/FX/P_Greystone_Primary_Impact.P_Greystone_Primary_Impact");
	ContactSparks = TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(Sparks));
	BodySparks = TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(Sparks));
	SlideSparks = TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(Sparks));
	StabImpact = TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(Impact));

	// 1) wide side shot while the boss winds up, 2) close side shot at the kick with a little hand-held shake, 3) tracking shot of the slide
	FRRCineShot Wide;
	Wide.StartTime = 0.0f; Wide.Side = 560.0f; Wide.Up = 140.0f; Wide.FOV = 50.0f; Wide.DriftCm = 45.0f; Wide.BlendIn = 0.45f;
	FRRCineShot Close;
	Close.StartTime = 0.75f; Close.Side = 330.0f; Close.Along = -120.0f; Close.Up = 110.0f; Close.FOV = 38.0f; Close.DriftCm = 25.0f; Close.BlendIn = 0.2f; Close.Shake = 1.4f;
	FRRCineShot Track;
	Track.StartTime = 1.15f; Track.Side = 400.0f; Track.Up = 95.0f; Track.FOV = 44.0f; Track.DriftCm = 20.0f; Track.BlendIn = 0.3f; Track.bFollowDefender = true; Track.Shake = 0.6f;
	Shots = { Wide, Close, Track };
}
