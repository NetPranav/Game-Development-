// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRCinematicDirector.h"
#include "RRCinematicSequence.h"
#include "RRGame.h"
#include "RRBoss.h"
#include "RREnemyDirector.h"
#include "AAACombatComponent.h"
#include "../../AAAAbility/Public/AAAMoverUtils.h" // two modules ship an AAAMoverUtils.h; we want the AAAAbility one
#include "AAAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	float Ease(float T) { T = FMath::Clamp(T, 0.0f, 1.0f); return T * T * (3.0f - 2.0f * T); }
}

void URRCinematicDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BossBeginHandle = FRRBossEvents::OnSpecialAttackBegin().AddUObject(this, &URRCinematicDirector::OnBossSpecialBegin);
	DeathblowHandle = FRRBossEvents::OnDeathblow().AddWeakLambda(this, [this](ARRBoss* Boss)
	{
		if (Boss && Boss->GetWorld() == GetWorld()) { PlayMoment(0.2f, 0.9f); }
	});
}

void URRCinematicDirector::PlayMoment(float Dilation, float RealSeconds)
{
	if (bActive || !GetWorld()) return;
	UGameplayStatics::SetGlobalTimeDilation(this, Dilation);
	MomentEnd = FPlatformTime::Seconds() + RealSeconds;
	bMoment = true;
}

void URRCinematicDirector::Deinitialize()
{
	FRRBossEvents::OnSpecialAttackBegin().Remove(BossBeginHandle);
	FRRBossEvents::OnDeathblow().Remove(DeathblowHandle);
	if (bMoment && GetWorld()) { UGameplayStatics::SetGlobalTimeDilation(this, 1.0f); bMoment = false; }
	RestoreWorld();
	Super::Deinitialize();
}

UAnimInstance* URRCinematicDirector_FindAnim(AActor* Actor)
{
	if (!Actor) return nullptr;
	if (UAAACombatComponent* Combat = Actor->FindComponentByClass<UAAACombatComponent>())
	{
		return Combat->FindAnimInstance();
	}
	return nullptr;
}

USkeletalMeshComponent* URRCinematicDirector::FindVisibleMesh(AActor* Actor) const
{
	if (!Actor) return nullptr;
	TInlineComponentArray<UChildActorComponent*> Children;
	Actor->GetComponents(Children);
	for (UChildActorComponent* Child : Children)
	{
		if (Child && Child->GetName().Contains(TEXT("VisualOverride")))
		{
			if (AActor* Display = Child->GetChildActor())
			{
				if (USkeletalMeshComponent* M = Display->FindComponentByClass<USkeletalMeshComponent>())
				{
					if (M->GetSkeletalMeshAsset()) return M;
				}
			}
		}
	}
	return Actor->FindComponentByClass<USkeletalMeshComponent>();
}

void URRCinematicDirector::SpawnEffectAttached(UParticleSystem* Effect, AActor* Target, FName Bone) const
{
	if (!Effect) return;
	if (USkeletalMeshComponent* Mesh = FindVisibleMesh(Target))
	{
		UGameplayStatics::SpawnEmitterAttached(Effect, Mesh, Mesh->DoesSocketExist(Bone) ? Bone : NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
}

float URRCinematicDirector::PlayAnimOnActor(AActor* Actor, const FRRCineAnim& Anim) const
{
	UAnimInstance* AnimInst = URRCinematicDirector_FindAnim(Actor);
	if (!AnimInst) return 0.0f;
	if (UAnimMontage* M = Anim.Montage.LoadSynchronous())
	{
		return AnimInst->Montage_Play(M, Anim.PlayRate);
	}
	if (UAnimSequence* S = Anim.Sequence.LoadSynchronous())
	{
		if (UAnimMontage* Dyn = AnimInst->PlaySlotAnimationAsDynamicMontage(S, FName("DefaultSlot"), 0.1f, 0.2f, Anim.PlayRate))
		{
			return Dyn->GetPlayLength();
		}
	}
	return 0.0f;
}

void URRCinematicDirector::OnBossSpecialBegin(ARRBoss* Boss, AActor* Target, float SecondsToContact)
{
	if (bActive || !Boss || !Target) return;
	const APawn* TargetPawn = Cast<APawn>(Target);
	if (!TargetPawn || !Cast<APlayerController>(TargetPawn->GetController())) return;

	URRCinematicSequence* Seq = NewObject<URRCinematicSequence>(this);
	PlaySequence(Seq, Boss, Target, SecondsToContact);
}

bool URRCinematicDirector::PlaySequence(URRCinematicSequence* Sequence, AActor* InAttacker, AActor* InDefender, float SecondsToContact)
{
	if (bActive || !Sequence || !InAttacker || !InDefender || !GetWorld()) return false;

	APawn* DefPawn = Cast<APawn>(InDefender);
	APlayerController* PC = DefPawn ? Cast<APlayerController>(DefPawn->GetController()) : GetWorld()->GetFirstPlayerController();
	if (!PC) return false;

	Active = DuplicateObject<URRCinematicSequence>(Sequence, this); // shot times are shifted to the real contact time below
	Attacker = InAttacker;
	Defender = InDefender;

	Axis = (InDefender->GetActorLocation() - InAttacker->GetActorLocation());
	Axis.Z = 0.0f;
	Axis = Axis.GetSafeNormal();
	if (Axis.IsNearlyZero()) Axis = InAttacker->GetActorForwardVector();
	Right = FVector::CrossProduct(FVector::UpVector, Axis).GetSafeNormal();
	StartDefenderLoc = InDefender->GetActorLocation();

	Time = 0.0;
	ContactTime = SecondsToContact >= 0.0f ? SecondsToContact : Sequence->DefaultLeadInSeconds;
	EndTime = ContactTime + Sequence->SlideDuration + Sequence->HoldAfterSlide;
	for (int32 i = 1; i < Active->Shots.Num(); ++i)
	{
		Active->Shots[i].StartTime += ContactTime - Sequence->DefaultLeadInSeconds;
	}
	bSlideAnimPlayed = bStopAnimPlayed = false;
	bContactDone = bSlideStarted = bTimeDilated = false;
	DilationRestoreAt = -1.0f;
	NextSlideSparkAt = 0.0f;

	// camera starts exactly where the gameplay camera is, so the first shot blends in smoothly
	FVector VLoc; FRotator VRot;
	PC->GetPlayerViewPoint(VLoc, VRot);
	LastCameraLoc = VLoc; LastCameraRot = VRot;
	LastCameraFOV = PC->PlayerCameraManager ? PC->PlayerCameraManager->GetFOVAngle() : 90.0f;
	bHaveLastCamera = true;
	PreviousViewTarget = PC->GetViewTarget();

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACameraActor* Cam = GetWorld()->SpawnActor<ACameraActor>(VLoc, VRot, Params);
	if (!Cam) { Active = nullptr; return false; }
	Cam->GetCameraComponent()->bConstrainAspectRatio = false;
	Cam->GetCameraComponent()->SetFieldOfView(LastCameraFOV);
	Camera = Cam;
	PC->SetViewTargetWithBlend(Cam, 0.0f);
	PC->SetCinematicMode(true, false, false, true, true);

	// the defender turns to face the attacker and braces
	FRotator Face = (-Axis).Rotation();
	Face.Pitch = 0.0f; Face.Roll = 0.0f;
	InDefender->SetActorRotation(Face);
	PlayAnimOnActor(InDefender, Sequence->DefenderBlockAnim);
	if (Sequence->AttackerAnim.Montage.ToSoftObjectPath().IsValid() || Sequence->AttackerAnim.Sequence.ToSoftObjectPath().IsValid())
	{
		PlayAnimOnActor(InAttacker, Sequence->AttackerAnim);
	}

	bActive = true;
	OnCinematicStarted.Broadcast();
	UE_LOG(LogRR, Log, TEXT("Cinematic started: contact in %.2fs, ends at %.2fs"), ContactTime, EndTime);
	return true;
}

void URRCinematicDirector::DoContact()
{
	bContactDone = true;
	AActor* Att = Attacker.Get();
	AActor* Def = Defender.Get();
	if (!Active || !Att || !Def) { Abort(); return; }

	const float Dist = FVector::Dist2D(Att->GetActorLocation(), Def->GetActorLocation());
	if (Dist > Active->CancelDistance)
	{
		UE_LOG(LogRR, Log, TEXT("Cinematic cancelled: defender out of reach (%.0f cm)"), Dist);
		Abort();
		return;
	}

	// slow-mo pulse + the kick lands
	UGameplayStatics::SetGlobalTimeDilation(this, Active->ContactTimeDilation);
	bTimeDilated = true;
	DilationRestoreAt = (float)Time + Active->ContactDilationSeconds;

	PlayAnimOnActor(Def, Active->DefenderImpactAnim);

	const FVector Chest = (Att->GetActorLocation() + Def->GetActorLocation()) * 0.5f + FVector(0, 0, 80.0f);
	if (UParticleSystem* Sparks = Active->ContactSparks.LoadSynchronous())
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, Sparks, Chest, FRotator::ZeroRotator, FVector(1.4f), true);
	}
	SpawnEffectAttached(Active->BodySparks.LoadSynchronous(), Def, Active->BodySparkBone);
	if (USoundBase* Snd = Active->ContactSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd, Chest);
	}

	// extra posture damage
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Def))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const float Poise = ASC->GetNumericAttribute(UAAAAttributeSet::GetPoiseAttribute());
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetPoiseAttribute(), FMath::Max(0.0f, Poise - Active->DefenderPoiseDamage));
		}
	}
	StartSlide();
}

void URRCinematicDirector::StartSlide()
{
	AActor* Def = Defender.Get();
	if (!Def || !Active) return;
	bSlideStarted = true;
	StartDefenderLoc = Def->GetActorLocation();
	UAAAMoverUtils::ApplyVelocityBurst(Def, Axis * (Active->SlideDistance / FMath::Max(0.1f, Active->SlideDuration)) * 2.4f, Active->SlideDuration);
}

void URRCinematicDirector::Tick(float DeltaTime)
{
	const float RealDelta = FMath::Min(FApp::GetDeltaTime(), 0.1f);
	if (!bActive)
	{
		// deathblow "moment": brief slow motion with the letterbox half in
		if (bMoment && FPlatformTime::Seconds() >= MomentEnd)
		{
			bMoment = false;
			UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
		}
		BarAlpha = FMath::FInterpConstantTo(BarAlpha, bMoment ? 0.6f : 0.0f, RealDelta, bMoment ? 6.0f : 2.0f);
		return;
	}

	Time += RealDelta;
	const bool bLeaving = Time > (EndTime - 0.3f);
	BarAlpha = FMath::FInterpConstantTo(BarAlpha, bLeaving ? 0.0f : 1.0f, RealDelta, 3.5f);

	if (!Attacker.IsValid() || !Defender.IsValid() || !Active)
	{
		Abort();
		return;
	}

	if (!bContactDone && Time >= ContactTime)
	{
		DoContact();
		if (!bActive) return;
	}

	if (bTimeDilated && DilationRestoreAt > 0.0f && Time >= DilationRestoreAt)
	{
		UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
		bTimeDilated = false;
	}

	if (bContactDone)
	{
		AActor* Def = Defender.Get();
		const float Since = (float)(Time - ContactTime);
		if (!bSlideAnimPlayed && Since >= 0.30f)
		{
			bSlideAnimPlayed = true;
			PlayAnimOnActor(Def, Active->DefenderSlideAnim);
		}
		if (Since < Active->SlideDuration && Since >= NextSlideSparkAt)
		{
			NextSlideSparkAt = Since + 0.12f;
			if (UParticleSystem* S = Active->SlideSparks.LoadSynchronous())
			{
				UGameplayStatics::SpawnEmitterAtLocation(this, S, Def->GetActorLocation() - FVector(0, 0, 85.0f), (-Axis).Rotation(), FVector(0.8f), true);
			}
		}
		if (!bStopAnimPlayed && Since >= Active->SlideDuration)
		{
			bStopAnimPlayed = true;
			PlayAnimOnActor(Def, Active->DefenderStopAnim);
			const FVector Tip = Def->GetActorLocation() + Def->GetActorForwardVector() * 90.0f - FVector(0, 0, 90.0f);
			if (UParticleSystem* S = Active->StabImpact.LoadSynchronous())
			{
				UGameplayStatics::SpawnEmitterAtLocation(this, S, Tip, FRotator::ZeroRotator, FVector(1.2f), true);
			}
			if (USoundBase* Snd = Active->StabSound.LoadSynchronous())
			{
				UGameplayStatics::PlaySoundAtLocation(this, Snd, Tip);
			}
		}
	}

	UpdateCamera(RealDelta);

	if (Time >= EndTime)
	{
		Finish();
	}
}

bool URRCinematicDirector::ComputeShot(int32 Index, float T, FVector& OutLoc, FRotator& OutRot, float& OutFOV) const
{
	AActor* Att = Attacker.Get();
	AActor* Def = Defender.Get();
	if (!Active || !Active->Shots.IsValidIndex(Index) || !Att || !Def) return false;
	const FRRCineShot& S = Active->Shots[Index];

	const FVector Focus = S.bFollowDefender ? Def->GetActorLocation() : (Att->GetActorLocation() + Def->GetActorLocation()) * 0.5f;
	FVector Pos = Focus + Right * S.Side + Axis * S.Along + FVector(0, 0, S.Up);
	const float Life = FMath::Max(0.0f, T - S.StartTime);
	const FVector LookAt = Focus + FVector(0, 0, S.LookAtHeight);
	Pos += (LookAt - Pos).GetSafeNormal() * S.DriftCm * FMath::Min(Life / 1.5f, 1.0f);
	if (S.Shake > 0.0f)
	{
		Pos += FVector(0, FMath::Sin(T * 17.0f), FMath::Sin(T * 23.0f + 1.3f)) * S.Shake;
	}
	OutLoc = Pos;
	OutRot = (LookAt - Pos).Rotation();
	OutFOV = S.FOV;
	return true;
}

void URRCinematicDirector::UpdateCamera(float RealDelta)
{
	ACameraActor* Cam = Camera.Get();
	if (!Cam || !Active || Active->Shots.Num() == 0) return;

	int32 Index = 0;
	for (int32 i = 0; i < Active->Shots.Num(); ++i)
	{
		if (Time >= Active->Shots[i].StartTime) Index = i;
	}
	FVector Loc; FRotator Rot; float FOV;
	if (!ComputeShot(Index, (float)Time, Loc, Rot, FOV)) return;

	const FRRCineShot& Shot = Active->Shots[Index];
	const float A = Shot.BlendIn > 0.001f ? Ease(((float)Time - Shot.StartTime) / Shot.BlendIn) : 1.0f;
	if (A < 1.0f)
	{
		FVector PLoc = LastCameraLoc; FRotator PRot = LastCameraRot; float PFOV = LastCameraFOV;
		if (Index > 0) { ComputeShot(Index - 1, (float)Time, PLoc, PRot, PFOV); }
		Loc = FMath::Lerp(PLoc, Loc, A);
		Rot = FQuat::Slerp(PRot.Quaternion(), Rot.Quaternion(), A).Rotator();
		FOV = FMath::Lerp(PFOV, FOV, A);
	}
	Cam->SetActorLocationAndRotation(Loc, Rot);
	static float LastLog = -1.0f;
	if ((float)Time - LastLog > 0.25f || Time < 0.05)
	{
		LastLog = (float)Time;
		UE_LOG(LogRR, VeryVerbose, TEXT("CineCam t=%.2f shot=%d cam=%s rot=%s fov=%.0f att=%s def=%s"), (float)Time, Index, *Loc.ToString(), *Rot.ToString(), FOV,
			Attacker.IsValid() ? *Attacker->GetActorLocation().ToString() : TEXT("-"), Defender.IsValid() ? *Defender->GetActorLocation().ToString() : TEXT("-"));
	}
	Cam->GetCameraComponent()->SetFieldOfView(FOV);
}

void URRCinematicDirector::Abort()
{
	if (bActive) Finish();
}

void URRCinematicDirector::RestoreWorld()
{
	if (bTimeDilated)
	{
		if (GetWorld()) UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
		bTimeDilated = false;
	}
}

void URRCinematicDirector::Finish()
{
	bActive = false;
	RestoreWorld();

	APawn* DefPawn = Cast<APawn>(Defender.Get());
	APlayerController* PC = DefPawn ? Cast<APlayerController>(DefPawn->GetController()) : (GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
	if (PC)
	{
		PC->SetCinematicMode(false, false, false, true, true);
		AActor* Back = PreviousViewTarget.Get() ? PreviousViewTarget.Get() : (AActor*)DefPawn;
		if (Back)
		{
			PC->SetViewTargetWithBlend(Back, Active ? Active->BlendOutSeconds : 0.5f, VTBlend_EaseInOut);
		}
	}
	if (UAnimInstance* AnimInst = URRCinematicDirector_FindAnim(Defender.Get()))
	{
		AnimInst->Montage_Stop(0.35f);
	}
	if (ACameraActor* Cam = Camera.Get())
	{
		Cam->SetLifeSpan((Active ? Active->BlendOutSeconds : 0.5f) + 0.2f);
	}
	Camera.Reset();
	Active = nullptr;
	OnCinematicFinished.Broadcast();
	UE_LOG(LogRR, Log, TEXT("Cinematic finished"));
}

// ---------------------------------------------------------------------------------------------
// Console helpers
// ---------------------------------------------------------------------------------------------
namespace
{
	FAutoConsoleCommandWithWorldAndArgs GRRCinePlay(
		TEXT("rr.cine.play"), TEXT("Plays the special-attack cinematic between the boss and the player (test)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			const float Delay = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.0f;
			TWeakObjectPtr<UWorld> W(World);
			auto Run = [W]()
			{
				UWorld* Wld = W.Get();
				if (!Wld) return;
				URRCinematicDirector* Dir = Wld->GetSubsystem<URRCinematicDirector>();
				APlayerController* PC = Wld->GetFirstPlayerController();
				URREnemyDirector* Enemies = Wld->GetSubsystem<URREnemyDirector>();
				if (!Dir || !PC || !PC->GetPawn() || !Enemies) return;
				for (ARREnemy* E : Enemies->GetLivingEnemies())
				{
					if (E)
					{
						// test helper: stand the boss in front of the player so the framing is meaningful
						const FVector P = PC->GetPawn()->GetActorLocation();
						const FVector Fwd = PC->GetPawn()->GetActorForwardVector().GetSafeNormal2D();
						E->SetActorLocationAndRotation(P + Fwd * 230.0f, (-Fwd).Rotation());
						Dir->PlaySequence(NewObject<URRCinematicSequence>(Dir), E, PC->GetPawn(), 0.9f);
						return;
					}
				}
			};
			if (Delay > 0.0f) { FTimerHandle H; World->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda(Run), Delay, false); } else { Run(); }
		}));

	FAutoConsoleCommandWithWorldAndArgs GRRCineAbort(
		TEXT("rr.cine.abort"), TEXT("Aborts the running cinematic"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (World) { if (URRCinematicDirector* Dir = World->GetSubsystem<URRCinematicDirector>()) Dir->Abort(); }
		}));
}
