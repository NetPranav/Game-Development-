// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/AAAAGameModeBase.h"
#include "RREnemy.h"
#include "RRTypes.h"
#include "RRGameMode.generated.h"

class UAAAWeaponSettings;

UENUM(BlueprintType)
enum class ERRMatchState : uint8
{
	PreMatch,
	InProgress,
	Won,
	Lost
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChangedSignature, ERRMatchState, NewState);

/** Per-life statistics shown on the defeat screen. */
USTRUCT(BlueprintType)
struct FRRRunStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") float DamageDealt = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") float DamageTaken = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") int32 Kills = 0;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") float TimeSeconds = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") int32 Scrap = 0;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") int32 Deaths = 0;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") int32 SessionScrap = 0;
	UPROPERTY(BlueprintReadOnly, Category = "RR|Stats") FString WeaponName;
};

UCLASS()
class RRGAME_API ARRGameMode : public AAAAAGameModeBase
{
	GENERATED_BODY()

public:
	ARRGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	ERRMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	float GetMatchTimeSeconds() const { return MatchTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	int32 GetEnemiesKilled() const { return EnemiesKilled; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	int32 GetEnemiesTotal() const { return EnemiesTotal; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	FRRRewardResult GetLastReward() const { return LastReward; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	bool IsRewardPending() const { return bRewardPending; }

	UFUNCTION(BlueprintCallable, Category = "RR|Match")
	void RestartMatch();

	/** Revives the player at the start point, clears the arena and starts a fresh countdown. */
	UFUNCTION(BlueprintCallable, Category = "RR|Match")
	void RespawnPlayer();

	/** Stats of the current / last life (frozen when the player dies). */
	UFUNCTION(BlueprintPure, Category = "RR|Match")
	FRRRunStats GetRunStats() const;

	/** True while the intro / main menu / loading screen is still in front of the match. */
	UFUNCTION(BlueprintPure, Category = "RR|Match")
	bool IsFrontEndPending() const { return bFrontEndPending; }

	UFUNCTION(BlueprintPure, Category = "RR|Match")
	bool IsPlayerReady() const { return bPlayerReady; }

	/** Called by the HUD when the loading screen is done: starts the first countdown. */
	UFUNCTION(BlueprintCallable, Category = "RR|Match")
	void BeginGameplay();

	/** Where the player (re)spawns: runtime override from rr.start.here, otherwise the level's PlayerStart (move it in the editor). */
	UFUNCTION(BlueprintPure, Category = "RR|Match")
	FTransform GetStartTransform() const;

	/** Moves the runtime respawn point (editor-placed PlayerStart is untouched). */
	UFUNCTION(BlueprintCallable, Category = "RR|Match")
	void SetRespawnPoint(const FTransform& NewPoint);

	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	// ---- duel platform (ARRArena placed in the level) ----
	class ARRArena* GetArena() const { return Arena.Get(); }
	/** The entry circle is live: pre-match, boss standing on the platform, not mid-entrance. */
	bool CanEnterArena() const;
	/** Called by the arena when the player held the entry circle: teleport in and start the duel. */
	void RequestArenaMatch(class ARRArena* InArena);
	/** Duel running (NPC spectators cheer, music is battle). */
	bool IsDuelActive() const { return MatchState == ERRMatchState::InProgress && Arena.IsValid(); }
	class ARRBoss* FindBoss() const;

	UPROPERTY(BlueprintAssignable, Category = "RR|Match")
	FOnMatchStateChangedSignature OnMatchStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	TSubclassOf<ARREnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	int32 EnemyCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	float SpawnRingRadius = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	float PreMatchSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	int32 Waves = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	FVector ArenaCenterOverride = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	bool bUseArenaCenterOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RR|Match")
	TObjectPtr<UAAAWeaponSettings> PlayerWeaponSettings;

protected:
	void ChangeMatchState(ERRMatchState NewState);
	void PollPlayerReady();
	void StartPreMatch();
	void StartWave();
	void SetupPlayerLoadout();

	/** One Game Animation Sample NPC per configured look, standing around the arena (URRSettings NPC section). */
	void SpawnAmbientNpcs(const FVector& Center);
	void SpawnArenaBoss(bool bEntrance);
	void StartArenaDuel();
	void ExitArena();
	void TeleportPawn(APawn* Pawn, const FVector& Location, const FRotator& Facing, bool bEffects);
	void SetMusic(uint8 State);
	TWeakObjectPtr<class ARRArena> Arena;
	bool bBossEntrancePlayed = false;
	bool bDuelStarting = false;
	FTimerHandle ArenaTimer;
	bool bAmbientSpawned = false;

	UFUNCTION()
	void OnEnemyDied(ARREnemy* Enemy, int32 RemainingLiving);

	UFUNCTION()
	void OnAllEnemiesDefeated();

	UFUNCTION()
	void OnPlayerDied();

	UFUNCTION()
	void OnPlayerHealthChanged(float Health, float MaxHealth, float PreviousHealth);

	void OnHitConfirmedNative(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage);
	int32 ComputeScrap() const;
	void ResetRunStats();

	UFUNCTION()
	void OnRewardResultReceived(const FRRRewardResult& Result);

	ERRMatchState MatchState = ERRMatchState::PreMatch;
	float MatchTimeSeconds = 0.0f;
	int32 EnemiesKilled = 0;
	int32 EnemiesTotal = 0;
	int32 CurrentWave = 0;

	FTimerHandle PollPlayerTimer;
	FTimerHandle MatchTimer;
	FTimerHandle PreMatchTimer;
	float PreMatchCountdown = 0.0f;

	bool bFrontEndPending = false;
	bool bPlayerReady = false;
	bool bMatchStarted = false;

	FRRRunStats Stats;
	int32 Deaths = 0;
	int32 SessionScrap = 0;
	FDelegateHandle HitConfirmedHandle;

	/** Pawn location minus PlayerStart location, captured at first spawn so respawns land the same way. */
	FVector SpawnOffset = FVector(0.0f, 0.0f, 0.0f);
	bool bHasRespawnOverride = false;
	FTransform RespawnOverride;

	FRRRewardResult LastReward;
	bool bRewardPending = false;
	bool bRewardOffline = false;

	void MatchTimerTick();
	void PreMatchTick();

#if !UE_BUILD_SHIPPING
	IConsoleCommand* CmdWin = nullptr;
	IConsoleCommand* CmdLose = nullptr;
	IConsoleCommand* CmdRestart = nullptr;
	IConsoleCommand* CmdStartHere = nullptr;
#endif
};
