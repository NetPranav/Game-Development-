// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRAudioDirector.h"
#include "RRGame.h"
#include "RRBoss.h"
#include "AAACombatEvents.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	USoundBase* Au_Load(const TCHAR* Path) { return LoadObject<USoundBase>(nullptr, Path); }
	USoundBase* Au_Pick(const TArray<TObjectPtr<USoundBase>>& Arr) { return Arr.Num() ? Arr[FMath::RandRange(0, Arr.Num() - 1)].Get() : nullptr; }
}

bool URRAudioDirector::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && World->GetNetMode() != NM_DedicatedServer && Super::ShouldCreateSubsystem(Outer);
}

void URRAudioDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MusicExplore = Au_Load(TEXT("/Game/RR/Audio/MUS_Explore_HotSprings.MUS_Explore_HotSprings"));
	MusicTension = Au_Load(TEXT("/Game/RR/Audio/MUS_Shrine_Tension.MUS_Shrine_Tension"));
	MusicBattle = Au_Load(TEXT("/Game/RR/Audio/MUS_Battle_DeterminedPursuit.MUS_Battle_DeterminedPursuit"));
	AmbienceWind = Au_Load(TEXT("/Game/RR/Audio/AMB_Wind.AMB_Wind"));
	SfxCrowd = Au_Load(TEXT("/Game/RR/Audio/SFX/SFX_CrowdShout.SFX_CrowdShout"));
	SfxEffortSwing = Au_Load(TEXT("/Game/ParagonGreystone/Audio/Cues/Greystone_Effort_Swing.Greystone_Effort_Swing"));
	SfxEffortCheer = Au_Load(TEXT("/Game/ParagonGreystone/Audio/Cues/Greystone_Effort_Cheer.Greystone_Effort_Cheer"));
	for (int32 i = 1; i <= 10; ++i)
	{
		if (USoundBase* S = Au_Load(*FString::Printf(TEXT("/Game/RR/Audio/SFX/SFX_Sword_%02d.SFX_Sword_%02d"), i, i))) SfxSwords.Add(S);
		if (USoundBase* C = Au_Load(*FString::Printf(TEXT("/Game/RR/Audio/SFX/SFX_Clash_%02d.SFX_Clash_%02d"), i, i))) SfxClashes.Add(C);
	}

	HitHandle = FAAACombatEvents::OnHitConfirmed().AddUObject(this, &URRAudioDirector::OnHitConfirmed);
	BlockHandle = FAAACombatEvents::OnBlocked().AddUObject(this, &URRAudioDirector::OnBlocked);
	ParryHandle = FAAACombatEvents::OnParried().AddUObject(this, &URRAudioDirector::OnParried);
	DeathblowHandle = FRRBossEvents::OnDeathblow().AddUObject(this, &URRAudioDirector::OnDeathblow);
	PerilousHandle = FRRBossEvents::OnPerilousWarning().AddUObject(this, &URRAudioDirector::OnPerilousWarning);

	if (!IConsoleManager::Get().FindConsoleObject(TEXT("rr.music")))
	{
		MusicCmd = IConsoleManager::Get().RegisterConsoleCommand(TEXT("rr.music"), TEXT("rr.music explore|tension|battle|victory|defeat|none"),
			FConsoleCommandWithArgsDelegate::CreateUObject(this, &URRAudioDirector::OnMusicCommand));
	}
}

void URRAudioDirector::Deinitialize()
{
	FAAACombatEvents::OnHitConfirmed().Remove(HitHandle);
	FAAACombatEvents::OnBlocked().Remove(BlockHandle);
	FAAACombatEvents::OnParried().Remove(ParryHandle);
	FRRBossEvents::OnDeathblow().Remove(DeathblowHandle);
	FRRBossEvents::OnPerilousWarning().Remove(PerilousHandle);
	if (MusicCmd) { IConsoleManager::Get().UnregisterConsoleObject(MusicCmd); MusicCmd = nullptr; }
	if (CurrentMusicComp) CurrentMusicComp->Stop();
	if (FadingMusicComp) FadingMusicComp->Stop();
	if (AmbienceComp) AmbienceComp->Stop();
	Super::Deinitialize();
}

void URRAudioDirector::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	SetMusicState(ERRMusicState::Explore);
}

TStatId URRAudioDirector::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URRAudioDirector, STATGROUP_Tickables);
}

void URRAudioDirector::Tick(float DeltaTime)
{
	if (!bAmbienceStarted && AmbienceWind && GetWorld() && GetWorld()->HasBegunPlay())
	{
		bAmbienceStarted = true;
		AmbienceComp = UGameplayStatics::CreateSound2D(this, AmbienceWind, 0.25f, 1.0f, 0.0f, nullptr, false, false);
		if (AmbienceComp) AmbienceComp->FadeIn(3.0f, 1.0f);
	}
}

ERRMusicState URRAudioDirector::GetMusicState() const
{
	return CurrentMusicState;
}

void URRAudioDirector::SetMusicState(ERRMusicState NewState)
{
	if (NewState == CurrentMusicState || !GetWorld()) return;
	CurrentMusicState = NewState;

	USoundBase* Track = nullptr;
	float Volume = 0.0f;
	switch (NewState)
	{
	case ERRMusicState::Explore: Track = MusicExplore; Volume = 0.35f; break;
	case ERRMusicState::Tension: Track = MusicTension; Volume = 0.45f; break;
	case ERRMusicState::Battle:  Track = MusicBattle;  Volume = 0.55f; break;
	case ERRMusicState::Victory: Track = MusicExplore; Volume = 0.45f; break;
	case ERRMusicState::Defeat:  Track = MusicTension; Volume = 0.3f;  break;
	default: break;
	}

	// same track (Explore <-> Victory): only change the level
	if (CurrentMusicComp && Track && CurrentMusicComp->GetSound() == Track)
	{
		CurrentMusicComp->AdjustVolume(1.5f, Volume / FMath::Max(0.01f, MusicVolume));
		return;
	}

	if (FadingMusicComp) { FadingMusicComp->Stop(); }
	FadingMusicComp = CurrentMusicComp;
	if (FadingMusicComp) { FadingMusicComp->FadeOut(2.0f, 0.0f); }
	CurrentMusicComp = nullptr;

	if (Track)
	{
		CurrentMusicComp = UGameplayStatics::CreateSound2D(this, Track, Volume, 1.0f, 0.0f, nullptr, false, false);
		if (CurrentMusicComp)
		{
			CurrentMusicComp->bIsUISound = true;   // keeps playing while the game is paused (menus)
			CurrentMusicComp->FadeIn(2.5f, 1.0f);
		}
		MusicVolume = Volume;
	}
	UE_LOG(LogRR, Log, TEXT("Music -> %d (%s)"), (int32)NewState, *GetNameSafe(Track));
}

void URRAudioDirector::PlayCrowd(float Volume)
{
	if (SfxCrowd) UGameplayStatics::PlaySound2D(this, SfxCrowd, Volume);
}

void URRAudioDirector::PlaySfxAt(USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (Sound && GetWorld()) UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Volume, Pitch);
}

void URRAudioDirector::OnMusicCommand(const TArray<FString>& Args)
{
	const FString A = Args.Num() ? Args[0].ToLower() : FString();
	const ERRMusicState S = A == TEXT("explore") ? ERRMusicState::Explore : A == TEXT("tension") ? ERRMusicState::Tension
		: A == TEXT("battle") ? ERRMusicState::Battle : A == TEXT("victory") ? ERRMusicState::Victory
		: A == TEXT("defeat") ? ERRMusicState::Defeat : ERRMusicState::None;
	SetMusicState(S);
}

void URRAudioDirector::OnHitConfirmed(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	if (!Victim || Victim->GetWorld() != GetWorld()) return;
	const double Now = FPlatformTime::Seconds();
	if (Now - LastHitTime < 0.06) return;
	LastHitTime = Now;
	const FVector Loc = Hit.ImpactPoint.IsNearlyZero() ? Victim->GetActorLocation() : FVector(Hit.ImpactPoint);
	PlaySfxAt(Au_Pick(SfxSwords), Loc, bHeavy ? 1.0f : 0.9f, FMath::FRandRange(0.92f, 1.08f));
}

void URRAudioDirector::OnBlocked(AActor* Attacker, AActor* Victim)
{
	if (!Victim || Victim->GetWorld() != GetWorld()) return;
	PlaySfxAt(Au_Pick(SfxClashes), Victim->GetActorLocation(), 0.8f, FMath::FRandRange(0.95f, 1.05f));
}

void URRAudioDirector::OnParried(AActor* Attacker, AActor* Victim)
{
	AActor* At = Victim ? Victim : Attacker;
	if (!At || At->GetWorld() != GetWorld()) return;
	const FVector Loc = At->GetActorLocation() + FVector(0, 0, 80.0f);
	PlaySfxAt(Au_Pick(SfxClashes), Loc, 1.0f, 1.15f);
	PlaySfxAt(Au_Pick(SfxClashes), Loc, 1.0f, 0.85f);
}

void URRAudioDirector::OnDeathblow(ARRBoss* Boss)
{
	if (!Boss || Boss->GetWorld() != GetWorld()) return;
	PlayCrowd(0.8f);
	PlaySfxAt(Au_Pick(SfxClashes), Boss->GetActorLocation(), 1.0f, 0.7f);
}

void URRAudioDirector::OnPerilousWarning(ARRBoss* Boss, ERRBossAttackKind Kind, float WarningSeconds)
{
	if (!Boss || Boss->GetWorld() != GetWorld()) return;
	PlaySfxAt(SfxEffortSwing, Boss->GetActorLocation(), 1.0f, 1.0f);
}
