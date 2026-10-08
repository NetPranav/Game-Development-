// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRGameMode.h"
#include "RRGame.h"
#include "RRRobotVisualComponent.h"
#include "RRHUD.h"
#include "GameFramework/AAAAPlayerController.h"
#include "GameFramework/AAAAPlayerState.h"
#include "GameFramework/AAAAGameState.h"
#include "RRLoadoutComponent.h"
#include "RRSuiBridge.h"
#include "RREnemyDirector.h"
#include "AAAAttributeComponent.h"
#include "AAACombatComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AAAAttributeSet.h"
#include "AAACombatEvents.h"
#include "AAAGameplayTags.h"
#include "Engine/HitResult.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "RRSettings.h"
#include "RRBoss.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

ARRGameMode::ARRGameMode()
{
	PlayerControllerClass = AAAAAPlayerController::StaticClass();
	PlayerStateClass = AAAAAPlayerState::StaticClass();
	GameStateClass = AAAAAGameState::StaticClass();
	HUDClass = ARRHUD::StaticClass();
}

void ARRGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Intro -> main menu -> loading screen sits in front of the first match; tests skip it with -RRSkipMenu.
	bFrontEndPending = !FParse::Param(FCommandLine::Get(), TEXT("RRSkipMenu")) && GetNetMode() != NM_DedicatedServer;

	if (HasAuthority())
	{
		HitConfirmedHandle = FAAACombatEvents::OnHitConfirmed().AddUObject(this, &ARRGameMode::OnHitConfirmedNative);
		GetWorld()->GetTimerManager().SetTimer(PollPlayerTimer, this, &ARRGameMode::PollPlayerReady, 0.5f, true);

		if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
		{
			Director->OnAllEnemiesDefeated.AddDynamic(this, &ARRGameMode::OnAllEnemiesDefeated);
			Director->OnEnemyDied.AddDynamic(this, &ARRGameMode::OnEnemyDied);
		}

#if !UE_BUILD_SHIPPING
		CmdWin = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("rr.win"), TEXT(""), FConsoleCommandDelegate::CreateLambda([this]() { ChangeMatchState(ERRMatchState::Won); OnAllEnemiesDefeated(); }), ECVF_Cheat);
		CmdLose = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("rr.lose"), TEXT(""), FConsoleCommandDelegate::CreateLambda([this]() { OnPlayerDied(); }), ECVF_Cheat);
		CmdRestart = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("rr.match.restart"), TEXT(""), FConsoleCommandDelegate::CreateUObject(this, &ARRGameMode::RestartMatch), ECVF_Cheat);
		CmdStartHere = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("rr.start.here"), TEXT("Use the player's current location as the respawn point"),
			FConsoleCommandDelegate::CreateLambda([this]()
			{
				if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					if (APawn* P = PC->GetPawn())
					{
						SetRespawnPoint(P->GetActorTransform());
						UE_LOG(LogRR, Log, TEXT("Respawn point set to %s"), *P->GetActorLocation().ToString());
					}
				}
			}), ECVF_Cheat);
#endif
	}
}

void ARRGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollPlayerTimer);
		World->GetTimerManager().ClearTimer(MatchTimer);
		World->GetTimerManager().ClearTimer(PreMatchTimer);
	}
	FAAACombatEvents::OnHitConfirmed().Remove(HitConfirmedHandle);
	HitConfirmedHandle.Reset();

	Super::EndPlay(EndPlayReason);
#if !UE_BUILD_SHIPPING
	if (CmdWin) { IConsoleManager::Get().UnregisterConsoleObject(CmdWin); CmdWin = nullptr; }
	if (CmdLose) { IConsoleManager::Get().UnregisterConsoleObject(CmdLose); CmdLose = nullptr; }
	if (CmdRestart) { IConsoleManager::Get().UnregisterConsoleObject(CmdRestart); CmdRestart = nullptr; }
	if (CmdStartHere) { IConsoleManager::Get().UnregisterConsoleObject(CmdStartHere); CmdStartHere = nullptr; }
#endif
}

void ARRGameMode::ChangeMatchState(ERRMatchState NewState)
{
	if (MatchState != NewState)
	{
		MatchState = NewState;
		UE_LOG(LogRR, Log, TEXT("Match state -> %d (enemies killed %d)"), (int32)NewState, EnemiesKilled);
		OnMatchStateChanged.Broadcast(MatchState);
	}
}

void ARRGameMode::PollPlayerReady()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;

	GetWorld()->GetTimerManager().ClearTimer(PollPlayerTimer);

	{
		FVector StartLoc = FVector::ZeroVector;
		if (AActor* Start = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
		{
			StartLoc = Start->GetActorLocation();
		}
		SpawnOffset = PlayerPawn->GetActorLocation() - StartLoc;
		if (SpawnOffset.Size2D() > 200.0f)
		{
			SpawnOffset = FVector(0.0f, 0.0f, SpawnOffset.Z); // spawned elsewhere (collision push-out): only keep the height
		}
	}

	URRLoadoutComponent* Loadout = PlayerPawn->FindComponentByClass<URRLoadoutComponent>();
	if (!Loadout)
	{
		Loadout = NewObject<URRLoadoutComponent>(PlayerPawn);
		Loadout->RegisterComponent();
	}

	if (!PlayerPawn->FindComponentByClass<URRRobotVisualComponent>())
	{
		URRRobotVisualComponent* Visual = NewObject<URRRobotVisualComponent>(PlayerPawn);
		Visual->RegisterComponent();
	}

	SetupPlayerLoadout();

	if (PlayerWeaponSettings)
	{
		if (UAAACombatComponent* Combat = PlayerPawn->FindComponentByClass<UAAACombatComponent>())
		{
			Combat->EquipWeapon(PlayerWeaponSettings);
		}
	}

	if (UAAAAttributeComponent* AttrComp = PlayerPawn->FindComponentByClass<UAAAAttributeComponent>())
	{
		AttrComp->OnDeath.AddUniqueDynamic(this, &ARRGameMode::OnPlayerDied);
		AttrComp->OnHealthChanged.AddUniqueDynamic(this, &ARRGameMode::OnPlayerHealthChanged);
	}

	bPlayerReady = true;
	if (!bFrontEndPending)
	{
		bMatchStarted = true;
		StartPreMatch();
	}
	else
	{
		ChangeMatchState(ERRMatchState::PreMatch);
	}
}

void ARRGameMode::BeginGameplay()
{
	bFrontEndPending = false;
	// The player may have wandered while the intro ran: put them on the start point.
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* P = PC->GetPawn())
		{
			const FTransform Start = GetStartTransform();
			if (FVector::Dist(P->GetActorLocation(), Start.GetLocation()) > 50.0f)
			{
				P->SetActorLocationAndRotation(Start.GetLocation(), Start.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
	if (bPlayerReady && !bMatchStarted)
	{
		bMatchStarted = true;
		StartPreMatch();
	}
}

void ARRGameMode::SetupPlayerLoadout()
{
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	if (!PlayerPawn) return;

	URRLoadoutComponent* Loadout = PlayerPawn->FindComponentByClass<URRLoadoutComponent>();
	if (!Loadout) return;

	FRRGearData Wep; Wep.Kind = ERRGearKind::Weapon; Wep.Name = TEXT("Rustic Blade"); Wep.Damage = 12; Wep.Level = 1; Wep.Id = TEXT("starter_wep");
	FRRGearData Arm; Arm.Kind = ERRGearKind::Armor; Arm.Name = TEXT("Padded Frame"); Arm.Armor = 8; Arm.Level = 1; Arm.Id = TEXT("starter_arm");

	URRSuiBridge* SuiBridge = GetGameInstance()->GetSubsystem<URRSuiBridge>();
	if (SuiBridge && SuiBridge->IsConnected())
	{
		TArray<FRRGearData> Owned = SuiBridge->GetOwnedGear();
		for (const FRRGearData& Gear : Owned)
		{
			if (Gear.Kind == ERRGearKind::Weapon && Gear.Damage > Wep.Damage) Wep = Gear;
			if (Gear.Kind == ERRGearKind::Armor && Gear.Armor > Arm.Armor) Arm = Gear;
		}
	}

	Loadout->Equip(Wep);
	Loadout->Equip(Arm);

	if (SuiBridge && SuiBridge->IsConnected())
	{
		FRRGearData BestAb; bool bFoundAb = false;
		TArray<FRRGearData> Owned = SuiBridge->GetOwnedGear();
		for (const FRRGearData& Gear : Owned)
		{
			if (Gear.Kind == ERRGearKind::Ability)
			{
				if (!bFoundAb || Gear.Power > BestAb.Power)
				{
					BestAb = Gear;
					bFoundAb = true;
				}
			}
		}
		if (bFoundAb) Loadout->Equip(BestAb);
		else Loadout->Unequip(ERRGearKind::Ability);
	}
}

void ARRGameMode::StartPreMatch()
{
	ChangeMatchState(ERRMatchState::PreMatch);
	ResetRunStats();
	PreMatchCountdown = PreMatchSeconds;
	CurrentWave = 0;
	EnemiesKilled = 0;
	EnemiesTotal = 0;
	MatchTimeSeconds = 0.0f;
	LastReward = FRRRewardResult();
	bRewardPending = false;
	bRewardOffline = false;

	GetWorld()->GetTimerManager().SetTimer(PreMatchTimer, this, &ARRGameMode::PreMatchTick, 1.0f, true);
}

void ARRGameMode::PreMatchTick()
{
	PreMatchCountdown -= 1.0f;
	if (PreMatchCountdown <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PreMatchTimer);
		StartWave();
		ChangeMatchState(ERRMatchState::InProgress);
		GetWorld()->GetTimerManager().SetTimer(MatchTimer, this, &ARRGameMode::MatchTimerTick, 1.0f, true);
	}
}

void ARRGameMode::StartWave()
{
	const URRSettings* Settings = URRSettings::Get();
	bool bIsBossMode = Settings && Settings->bBossMode;

	CurrentWave++;
	
	if (bIsBossMode)
	{
		Waves = 1;
		EnemiesTotal = 1;
	}
	else
	{
		int32 SpawnCount = EnemyCount + (CurrentWave - 1);
		EnemiesTotal += SpawnCount;
	}

	FVector Center = ArenaCenterOverride;
	if (!bUseArenaCenterOverride)
	{
		if (AActor* Start = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
		{
			Center = Start->GetActorLocation();
		}
	}

	URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>();
	if (!Director) return;
	
	if (bIsBossMode)
	{
		UClass* LoadedBossClass = Settings ? Settings->BossClass.LoadSynchronous() : nullptr;
		if (!LoadedBossClass)
		{
			UE_LOG(LogRR, Warning, TEXT("Failed to load BossClass from Settings, falling back to ARRBoss"));
			// Forward declare ARRBoss or include it
			// I'll need to include "RRBoss.h" at the top of RRGameMode.cpp
			// Will rely on the include I will add next.
			// However, since it's an ARREnemy subclass, it works with SpawnEnemy
		}
		
		FVector SpawnLoc = Center + FVector(SpawnRingRadius, 0.0f, 0.0f);
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLoc);
		FVector DirToCenter = (Center - SpawnLoc).GetSafeNormal2D();
		SpawnTransform.SetRotation(DirToCenter.Rotation().Quaternion());
		
		// The Boss Class will be passed down, if null fallback to ARREnemy.
		Director->SpawnEnemy(LoadedBossClass, SpawnTransform);
	}
	else
	{
		if (!EnemyClass) return;
		int32 SpawnCount = EnemyCount + (CurrentWave - 1);
		for (int32 i = 0; i < SpawnCount; ++i)
		{
			float Angle = (PI * 2.0f / SpawnCount) * i;
			FVector Offset(FMath::Cos(Angle) * SpawnRingRadius, FMath::Sin(Angle) * SpawnRingRadius, 0.0f);
			FVector SpawnLoc = Center + Offset;
			
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SpawnLoc);
			
			FVector DirToCenter = (Center - SpawnLoc).GetSafeNormal2D();
			SpawnTransform.SetRotation(DirToCenter.Rotation().Quaternion());

			Director->SpawnEnemy(EnemyClass, SpawnTransform);
		}
	}
}

void ARRGameMode::MatchTimerTick()
{
	MatchTimeSeconds += 1.0f;
}

void ARRGameMode::OnEnemyDied(ARREnemy* Enemy, int32 RemainingLiving)
{
	EnemiesKilled++;
}

void ARRGameMode::OnAllEnemiesDefeated()
{
	if (MatchState != ERRMatchState::InProgress) return;

	if (CurrentWave < Waves)
	{
		StartWave();
	}
	else
	{
		ChangeMatchState(ERRMatchState::Won);
		GetWorld()->GetTimerManager().ClearTimer(MatchTimer);

		URRSuiBridge* SuiBridge = GetGameInstance()->GetSubsystem<URRSuiBridge>();
		if (SuiBridge && SuiBridge->IsConnected())
		{
			bRewardPending = true;
			SuiBridge->OnRewardResult.AddUniqueDynamic(this, &ARRGameMode::OnRewardResultReceived);
			SuiBridge->RequestReward(true, EnemiesKilled);
		}
		else
		{
			bRewardOffline = true;
			FRRRewardResult FakeRes;
			FakeRes.bOk = false;
			FakeRes.Error = TEXT("Offline - no on-chain reward");
			LastReward = FakeRes;
		}
	}
}

void ARRGameMode::OnPlayerDied()
{
	if (MatchState == ERRMatchState::InProgress || MatchState == ERRMatchState::PreMatch)
	{
		Deaths++;
		Stats.Kills = EnemiesKilled;
		Stats.TimeSeconds = MatchTimeSeconds;
		Stats.Deaths = Deaths;
		Stats.Scrap = ComputeScrap();
		SessionScrap += Stats.Scrap;
		Stats.SessionScrap = SessionScrap;
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FRRGearData Wep;
			if (PC->GetPawn())
			{
				if (URRLoadoutComponent* L = PC->GetPawn()->FindComponentByClass<URRLoadoutComponent>())
				{
					if (L->GetEquipped(ERRGearKind::Weapon, Wep)) Stats.WeaponName = Wep.Name;
				}
			}
		}
		UE_LOG(LogRR, Log, TEXT("Player defeated: dealt %.0f taken %.0f kills %d scrap %d"), Stats.DamageDealt, Stats.DamageTaken, Stats.Kills, Stats.Scrap);

		ChangeMatchState(ERRMatchState::Lost);
		GetWorld()->GetTimerManager().ClearTimer(MatchTimer);
		GetWorld()->GetTimerManager().ClearTimer(PreMatchTimer);
	}
}

void ARRGameMode::OnPlayerHealthChanged(float Health, float MaxHealth, float PreviousHealth)
{
	if (MatchState == ERRMatchState::InProgress && PreviousHealth > Health)
	{
		Stats.DamageTaken += PreviousHealth - Health;
	}
}

void ARRGameMode::OnHitConfirmedNative(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC && Attacker && Attacker == PC->GetPawn() && MatchState == ERRMatchState::InProgress)
	{
		Stats.DamageDealt += FMath::Max(0.0f, Damage);
	}
}

int32 ARRGameMode::ComputeScrap() const
{
	return Stats.Kills * 50 + FMath::RoundToInt(Stats.DamageDealt * 0.5f);
}

void ARRGameMode::ResetRunStats()
{
	Stats = FRRRunStats();
	Stats.Deaths = Deaths;
	Stats.SessionScrap = SessionScrap;
}

FRRRunStats ARRGameMode::GetRunStats() const
{
	FRRRunStats Out = Stats;
	if (MatchState != ERRMatchState::Lost)
	{
		Out.Kills = EnemiesKilled;
		Out.TimeSeconds = MatchTimeSeconds;
		Out.Scrap = ComputeScrap();
	}
	return Out;
}

FTransform ARRGameMode::GetStartTransform() const
{
	if (bHasRespawnOverride)
	{
		return RespawnOverride;
	}
	if (AActor* Start = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
	{
		return FTransform(Start->GetActorRotation(), Start->GetActorLocation() + SpawnOffset);
	}
	return FTransform(FRotator::ZeroRotator, SpawnOffset);
}

void ARRGameMode::SetRespawnPoint(const FTransform& NewPoint)
{
	RespawnOverride = NewPoint;
	bHasRespawnOverride = true;
}

void ARRGameMode::RespawnPlayer()
{
	RestartMatch();
}

void ARRGameMode::OnRewardResultReceived(const FRRRewardResult& Result)
{
	LastReward = Result;
	bRewardPending = false;
	
	URRSuiBridge* SuiBridge = GetGameInstance()->GetSubsystem<URRSuiBridge>();
	if (SuiBridge)
	{
		SuiBridge->OnRewardResult.RemoveDynamic(this, &ARRGameMode::OnRewardResultReceived);
	}
}

void ARRGameMode::RestartMatch()
{
	if (!HasAuthority()) return;

	if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
	{
		TArray<ARREnemy*> Living = Director->GetLivingEnemies();
		for (ARREnemy* Enemy : Living)
		{
			if (Enemy) Enemy->Destroy();
		}
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		APawn* PlayerPawn = PC->GetPawn();
		
		if (UAAAAttributeComponent* AttrComp = PlayerPawn->FindComponentByClass<UAAAAttributeComponent>())
		{
			if (AttrComp->IsDead()) AttrComp->Revive();
		}

		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn)) ASC = ASI->GetAbilitySystemComponent();
		else if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(PlayerPawn->GetPlayerState())) ASC = PS_ASI->GetAbilitySystemComponent();

		if (ASC)
		{
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxHealthAttribute()));
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetStaminaAttribute(), ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxStaminaAttribute()));
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetPoiseAttribute(), ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxPoiseAttribute()));
		}

		if (UAAACombatComponent* Combat = PlayerPawn->FindComponentByClass<UAAACombatComponent>())
		{
			Combat->ClearState(FAAAGameplayTags::Get().State_Combat_Dead);
			Combat->ClearState(FAAAGameplayTags::Get().State_Combat_Staggered);
			Combat->SetBlocking(false);
			if (PlayerWeaponSettings) Combat->EquipWeapon(PlayerWeaponSettings);
		}
		
			// The player pawn disables its own input and tick on death (AAAAPlayerPawn::OnDeath): undo that.
		PlayerPawn->SetActorTickEnabled(true);
		PlayerPawn->EnableInput(PC);

		const FTransform Start = GetStartTransform();
		PlayerPawn->SetActorLocationAndRotation(Start.GetLocation(), Start.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		PC->SetControlRotation(Start.Rotator());
		UE_LOG(LogRR, Log, TEXT("Player respawned at %s"), *Start.GetLocation().ToString());
	}

	GetWorld()->GetTimerManager().ClearTimer(MatchTimer);
	GetWorld()->GetTimerManager().ClearTimer(PreMatchTimer);
	
	StartPreMatch();
}
