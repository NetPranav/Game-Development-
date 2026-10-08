// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RRCinematicSequence.generated.h"

class UAnimMontage;
class UAnimSequence;
class UParticleSystem;
class USoundBase;

/** One animation slot of a cinematic: a montage, or a plain sequence that is played as a dynamic montage. Drop a downloaded animation in here. */
USTRUCT(BlueprintType)
struct RRGAME_API FRRCineAnim
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Montage;

	/** Used when Montage is empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimSequence> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PlayRate = 1.0f;

	FRRCineAnim() {}
	explicit FRRCineAnim(const TCHAR* MontagePath) : Montage(FSoftObjectPath(MontagePath)) {}
};

/** One camera shot, expressed relative to the attacker -> defender axis. */
USTRUCT(BlueprintType)
struct RRGAME_API FRRCineShot
{
	GENERATED_BODY()

	/** Seconds after the cinematic started when this shot becomes active. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float StartTime = 0.0f;

	/** Distance of the camera to the side of the fight (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Side = 480.0f;

	/** Offset along the attacker -> defender axis, from the focus point (cm, positive = towards the defender). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Along = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Up = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float FOV = 50.0f;

	/** Height of the point the camera looks at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float LookAtHeight = 90.0f;

	/** Slow dolly towards the focus over the life of the shot (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float DriftCm = 30.0f;

	/** Seconds to blend from the previous shot (0 = cut). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float BlendIn = 0.25f;

	/** Focus on the defender instead of the midpoint (tracking shot). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	bool bFollowDefender = false;

	/** Hand-held shake amplitude (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Shake = 0.0f;
};

/**
 * Data driven special-attack cinematic. Defaults reproduce "robot braces with its sword, gets kicked, slides back and plants its
 * sword to stop" with the animations already in the project; assign your own animations in a Data Asset of this class
 * (or in URRSettings::SpecialSequence) to change it without code.
 */
UCLASS(BlueprintType)
class RRGAME_API URRCinematicSequence : public UDataAsset
{
	GENERATED_BODY()

public:
	URRCinematicSequence();

	// ---- animations ----
	/** Played on the attacker at the start (leave empty when the attacker already plays its own attack, like the boss does). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FRRCineAnim AttackerAnim;

	/** Defender braces with its sword before contact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FRRCineAnim DefenderBlockAnim;

	/** Defender gets hit (the kick). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FRRCineAnim DefenderImpactAnim;

	/** Defender slides backwards (looping animation, the actor itself is moved by the director). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FRRCineAnim DefenderSlideAnim;

	/** Pose held after the slide: sword planted in the ground. Replace with a dedicated animation when you have one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FRRCineAnim DefenderStopAnim;

	// ---- timing ----
	/** Used when the starter gives no time-to-contact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DefaultLeadInSeconds = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float SlideDuration = 0.8f;

	/** Distance the defender slides (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float SlideDistance = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float HoldAfterSlide = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BlendOutSeconds = 0.5f;

	/** Time dilation pulse at contact (global slow-mo). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float ContactTimeDilation = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float ContactDilationSeconds = 0.16f;

	/** If the defender is further than this from the attacker at contact, the scene is cancelled (clean dodge). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float CancelDistance = 420.0f;

	// ---- camera ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TArray<FRRCineShot> Shots;

	// ---- effects ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UParticleSystem> ContactSparks;

	/** Small sparks on the defender's body after contact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UParticleSystem> BodySparks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FName BodySparkBone = TEXT("spine_03");

	/** Sparks at the feet during the slide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UParticleSystem> SlideSparks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UParticleSystem> StabImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<USoundBase> ContactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<USoundBase> StabSound;

	// ---- gameplay ----
	/** Extra posture damage the defender takes at contact (the boss' own damage still applies). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float DefenderPoiseDamage = 25.0f;
};
