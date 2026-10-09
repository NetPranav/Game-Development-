# P13c - Audio director (music states + combat SFX)

Create ONLY these two new files (no other file may be edited):
- Source/RRGame/Public/RRAudioDirector.h
- Source/RRGame/Private/RRAudioDirector.cpp
Report to Docs/reports/P13c_report.md.

`URRAudioDirector : public UTickableWorldSubsystem` (module RRGame, macro RRGAME_API, include "Subsystems/WorldSubsystem.h").
Skip everything when `GetWorld()->GetNetMode() == NM_DedicatedServer` or the world is not a game world
(`ShouldCreateSubsystem`: return Outer world `IsGameWorld()`).

## Assets (load with `LoadObject<USoundBase>(nullptr, TEXT("/Game/RR/Audio/...")`; all SoundWaves already set to loop where noted)
- Music (looping): `/Game/RR/Audio/MUS_Explore_HotSprings.MUS_Explore_HotSprings` (calm koto),
  `/Game/RR/Audio/MUS_Shrine_Tension.MUS_Shrine_Tension` (tension), `/Game/RR/Audio/MUS_Battle_DeterminedPursuit.MUS_Battle_DeterminedPursuit` (battle).
- Ambience (looping): `/Game/RR/Audio/AMB_Wind.AMB_Wind`.
- SFX one-shots: `/Game/RR/Audio/SFX/SFX_Sword_01` .. `SFX_Sword_10` (sword hits), `/Game/RR/Audio/SFX/SFX_Clash_01` .. `SFX_Clash_10`
  (metal clash / block), `/Game/RR/Audio/SFX/SFX_CrowdShout`, and Paragon cues `/Game/ParagonGreystone/Audio/Cues/Greystone_Effort_Swing`,
  `/Game/ParagonGreystone/Audio/Cues/Greystone_Effort_Cheer`. Object path format is `/Game/Dir/Name.Name`.

## Public API
```cpp
UENUM(BlueprintType) enum class ERRMusicState : uint8 { None, Explore, Tension, Battle, Victory, Defeat };
UFUNCTION(BlueprintCallable) void SetMusicState(ERRMusicState NewState);   // crossfade (old FadeOut 2.0 s, new FadeIn 2.5 s)
UFUNCTION(BlueprintCallable) ERRMusicState GetMusicState() const;
UFUNCTION(BlueprintCallable) void PlayCrowd(float Volume = 0.6f);          // one-shot crowd shout (2D)
UFUNCTION(BlueprintCallable) void PlaySfxAt(USoundBase* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);
```
- Music uses two `UAudioComponent*` (UPROPERTY Transient) created with `UGameplayStatics::CreateSound2D(this, Sound, Volume, 1, 0, nullptr, /*persist across level*/ false, /*auto destroy*/ false)`;
  crossfade with `FadeOut(2.0f, 0.0f)` / `FadeIn(2.5f, Volume)`. Volumes: Explore 0.35, Tension 0.45, Battle 0.55.
  Victory = Explore at 0.45; Defeat = Tension at 0.3. None = fade out everything.
- Ambience wind: one 2D looping audio component at volume 0.25, started on first Tick when the world has begun play, always on.
- In `OnWorldBeginPlay` (override `virtual void OnWorldBeginPlay(UWorld& InWorld) override;`) start Explore.
- Console commands (`IConsoleManager::Get().RegisterConsoleCommand`, unregister in Deinitialize): `rr.music <explore|tension|battle|victory|defeat|none>`.

## Combat SFX (bind in Initialize, remove in Deinitialize)
Include "AAACombatEvents.h" (module AAACombat is already a dependency) and "RRBoss.h".
- `FAAACombatEvents::OnHitConfirmed()` (AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage):
  random SFX_Sword_XX at `Hit.ImpactPoint` (or Victim location if zero), volume 0.9 (1.0 if bHeavy), pitch random 0.92..1.08.
  Throttle: max one per 0.06 s.
- `FAAACombatEvents::OnBlocked()` (Attacker, Victim): random SFX_Clash at victim location, volume 0.8.
- `FAAACombatEvents::OnParried()` (Attacker, Victim): TWO clash sounds layered (one pitch 1.15, one 0.85), volume 1.0.
- `FRRBossEvents::OnDeathblow()` (ARRBoss*): PlayCrowd(0.8) + Greystone_Effort_Cheer at the boss? no - play a heavy clash at the boss location pitch 0.7.
- `FRRBossEvents::OnPerilousWarning()` (ARRBoss*, ERRBossAttackKind, float): Greystone_Effort_Swing at boss location.
Look at Source/RRGame/Public/RRBoss.h for the exact FRRBossEvents delegate signatures and Source/AAACombat/Public/AAACombatEvents.h for the others.
Use `UGameplayStatics::PlaySoundAtLocation`. Keep sounds attenuated by default engine settings (no attenuation asset needed).

Tick: only needed to start ambience once; `GetStatId` via RETURN_QUICK_DECLARE_CYCLE_STAT. Verify APIs in A:/UNREAL/UE_5.8/Engine/Source/Runtime/Engine/Classes/Kismet/GameplayStatics.h and Components/AudioComponent.h.
