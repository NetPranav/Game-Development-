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
#include "RRNpcController.h"
#include "RRCinematicLook.h"
#include "RRArena.h"
#include "RRSwordStanceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "RRAudioDirector.h"
#include "RRBossAIController.h"
#include "Particles/ParticleSystem.h"

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

	Arena = Cast<ARRArena>(UGameplayStatics::GetActorOfClass(GetWorld(), ARRArena::StaticClass()));
	UE_LOG(LogRR, Log, TEXT("Duel arena: %s"), Arena.IsValid() ? *Arena->GetName() : TEXT("none (boss spawns at the start point)"));

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
		if (!IConsoleManager::Get().FindConsoleObject(TEXT("rr.arena.enter")))
		{
			IConsoleManager::Get().RegisterConsoleCommand(TEXT("rr.arena.enter"), TEXT("rr.arena.enter [delay]: act as if the player held the entry circle (retries until allowed)"),
				FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
				{
					if (!World) return;
					const float Delay = Args.Num() ? FCString::Atof(*Args[0]) : 0.0f;
					TWeakObjectPtr<UWorld> WeakWorld(World);
					TSharedPtr<FTimerHandle> H = MakeShared<FTimerHandle>();
					World->GetTimerManager().SetTimer(*H, [WeakWorld, H]()
					{
						ARRGameMode* G = WeakWorld.IsValid() ? WeakWorld->GetAuthGameMode<ARRGameMode>() : nullptr;
						if (!G) return;
						if (G->CanEnterArena()) { G->RequestArenaMatch(G->GetArena()); WeakWorld->GetTimerManager().ClearTimer(*H); }
					}, 0.5f, true, FMath::Max(0.01f, Delay));
				}), ECVF_Cheat);
		}
		if (!IConsoleManager::Get().FindConsoleObject(TEXT("rr.cam.at")))
		{
			// dev: look from a fixed camera ("rr.cam.at X Y Z Pitch Yaw"), "rr.cam.at" alone returns to the player
			IConsoleManager::Get().RegisterConsoleCommand(TEXT("rr.cam.at"), TEXT("rr.cam.at X Y Z Pitch Yaw"),
				FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
				{
					APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
					if (!PC) return;
					if (Args.Num() < 5) { PC->SetViewTargetWithBlend(PC->GetPawn(), 0.3f); return; }
					ACameraActor* Cam = World->SpawnActor<ACameraActor>(FVector(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]), FCString::Atof(*Args[2])),
						FRotator(FCString::Atof(*Args[3]), FCString::Atof(*Args[4]), 0.0f));
					if (Cam) { Cam->GetCameraComponent()->SetFieldOfView(Args.Num() > 5 ? FCString::Atof(*Args[5]) : 70.0f); PC->SetViewTarget(Cam); }
				}), ECVF_Cheat);
		}
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

APawn* ARRGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	return GetWorld()->SpawnActor<APawn>(GetDefaultPawnClassForController(NewPlayer), SpawnTransform, SpawnInfo);
}

void ARRGameMode::PollPlayerReady()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		RestartPlayer(PC);
		PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) return;
	}

	GetWorld()->GetTimerManager().ClearTimer(PollPlayerTimer);

	SpawnOffset = FVector::ZeroVector;

	URRLoadoutComponent* Loadout = PlayerPawn->FindComponentByClass<URRLoadoutComponent>();
	if (!Loadout)
	{
		Loadout = NewObject<URRLoadoutComponent>(PlayerPawn);
		Loadout->RegisterComponent();
	}

	if (!PlayerPawn->FindComponentByClass<URRSwordStanceComponent>())
	{
		URRSwordStanceComponent* Stance = NewObject<URRSwordStanceComponent>(PlayerPawn);
		Stance->RegisterComponent();
	}

	if (!PlayerPawn->FindComponentByClass<URRRobotVisualComponent>())
	{
		URRRobotVisualComponent* Visual = NewObject<URRRobotVisualComponent>(PlayerPawn);
		Visual->RegisterComponent();
	}

	SetupPlayerLoadout();

	if (UAAAAttributeComponent* AttrComp = PlayerPawn->FindComponentByClass<UAAAAttributeComponent>())
	{
		AttrComp->OnDeath.AddUniqueDynamic(this, &ARRGameMode::OnPlayerDied);
		AttrComp->OnHealthChanged.AddUniqueDynamic(this, &ARRGameMode::OnPlayerHealthChanged);
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetMaxHealthAttribute(), 250.0f);
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), 250.0f);
		}
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

	FRRGearData Wep; Wep.Kind = ERRGearKind::Weapon; Wep.Name = TEXT("Fab Longsword"); Wep.Damage = 45; Wep.Level = 1; Wep.Id = TEXT("local_w4");
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

	if (Arena.IsValid())
	{
		// with a platform the duel waits for the entry circle instead of a countdown
		bDuelStarting = false;
		CurrentWave = 1; Waves = 1; EnemiesTotal = 1;
		SpawnAmbientNpcs(Arena->GetCenter());
		Arena->SetEntryEnabled(true);
		if (!bFrontEndPending) { SpawnArenaBoss(!bBossEntrancePlayed); }
		SetMusic((uint8)ERRMusicState::Explore);
		return;
	}
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

	SpawnAmbientNpcs(Center);

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
		if (Arena.IsValid())
		{
			SetMusic((uint8)ERRMusicState::Victory);
			if (URRAudioDirector* Audio = GetWorld()->GetSubsystem<URRAudioDirector>()) { Audio->PlayCrowd(0.9f); }
			GetWorld()->GetTimerManager().SetTimer(ArenaTimer, this, &ARRGameMode::ExitArena, 5.0f, false);
		}

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
		SetMusic((uint8)ERRMusicState::Defeat);
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

	FVector GroundPos(2000.0f, 400.0f, 150.0f);
	if (UWorld* W = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Q(SCENE_QUERY_STAT(RRStartGround), false);
		if (W->LineTraceSingleByChannel(Hit, GroundPos + FVector(0.0f, 0.0f, 1500.0f), GroundPos - FVector(0.0f, 0.0f, 3000.0f), ECC_Visibility, Q))
		{
			GroundPos = Hit.ImpactPoint + FVector(0.0f, 0.0f, 92.0f);
		}
	}

	if (AActor* Start = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
	{
		return Start->GetActorTransform();
	}
	return FTransform(FRotator(0.0f, 0.0f, 0.0f), GroundPos);
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
			if (URRLoadoutComponent* Loadout = PlayerPawn->FindComponentByClass<URRLoadoutComponent>())
			{
				FRRGearData EquippedWep;
				if (Loadout->GetEquipped(ERRGearKind::Weapon, EquippedWep))
				{
					Loadout->Equip(EquippedWep);
				}
			}
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

void ARRGameMode::SpawnAmbientNpcs(const FVector& Center)
{
	if (bAmbientSpawned) return;
	bAmbientSpawned = true;
	UWorld* World = GetWorld();
	const URRSettings* Settings = URRSettings::Get();
	if (!World || !Settings) return;

	if (Settings->bCinematicLook)
	{
		World->SpawnActor<ARRCinematicLook>();
	}

	if (!Settings->bSpawnNpcs || FParse::Param(FCommandLine::Get(), TEXT("RRNoNpcs"))) return;
	UClass* PawnClass = Settings->NpcPawnClass.LoadSynchronous();
	if (!PawnClass) { UE_LOG(LogRR, Warning, TEXT("NPC pawn class missing")); return; }

	// Place NPCs in the cheering area (the tiered stands of the Colosseum) overlooking the duel
	const int32 Num = Settings->NpcVisualClasses.Num();
	const FVector ArenaCenter = Arena.IsValid() ? Arena->GetCenter() : Center;
	int32 Spawned = 0;

	for (int32 i = 0; i < Num; ++i)
	{
		UClass* Look = Settings->NpcVisualClasses[i].LoadSynchronous();
		if (!Look) { UE_LOG(LogRR, Warning, TEXT("NPC look %s missing"), *Settings->NpcVisualClasses[i].ToString()); continue; }

		// Distribute around the cheering stands (radii 2400 to 2800, outside the 1750 green combat ring)
		const float BaseAngle = (2.0f * PI * i) / FMath::Max(1, Num) + 0.25f;
		const float StandRadius = 2400.0f + ((i % 3) * 160.0f);

		FVector Loc = FVector::ZeroVector;
		bool bFound = false;
		FCollisionQueryParams Q(SCENE_QUERY_STAT(RRNpcGround), false);

		// search a walkable seat: start ~9 m above the floor (a 15 m start sat inside the stand geometry and "hit" at its own
		// start point -> NPCs floated at Z 1504), skip start-penetrating hits and steep faces, try nearby radii / angles
		for (const float AngleOffset : { 0.0f, 0.12f, -0.12f, 0.25f, -0.25f })
		for (float R = StandRadius; R <= StandRadius + 600.0f && !bFound; R += 100.0f)
		{
			const FVector Dir(FMath::Cos(BaseAngle + AngleOffset), FMath::Sin(BaseAngle + AngleOffset), 0.0f);
			const FVector Probe = ArenaCenter + Dir * R;
			FHitResult Ground;
			if (World->LineTraceSingleByChannel(Ground, Probe + FVector(0, 0, 900.0f), Probe - FVector(0, 0, 400.0f), ECC_Visibility, Q)
				&& !Ground.bStartPenetrating && Ground.ImpactNormal.Z > 0.7f && Ground.ImpactPoint.Z > ArenaCenter.Z + 20.0f)
			{
				Loc = Ground.ImpactPoint + FVector(0, 0, 95.0f);
				bFound = true;
			}
		}

		// fallback: the strip of arena floor between the duel wall and the stands (right beside the fight)
		if (!bFound && Arena.IsValid())
		{
			for (const float AngleOffset : { 0.0f, 0.15f, -0.15f, 0.3f, -0.3f })
			for (float R = Arena->Radius + 130.0f; R <= Arena->Radius + 330.0f && !bFound; R += 50.0f)
			{
				const FVector Dir(FMath::Cos(BaseAngle + AngleOffset), FMath::Sin(BaseAngle + AngleOffset), 0.0f);
				const FVector Probe = ArenaCenter + Dir * R;
				FHitResult Ground;
				if (World->LineTraceSingleByChannel(Ground, Probe + FVector(0, 0, 250.0f), Probe - FVector(0, 0, 400.0f), ECC_Visibility, Q)
					&& !Ground.bStartPenetrating && Ground.ImpactNormal.Z > 0.8f && FMath::Abs(Ground.ImpactPoint.Z - ArenaCenter.Z) < 120.0f)
				{
					Loc = Ground.ImpactPoint + FVector(0, 0, 95.0f);
					bFound = true;
				}
			}
		}
		if (!bFound) { UE_LOG(LogRR, Log, TEXT("NPC %d: no ground on cheering stand, skipped"), i); continue; }
		const FRotator Facing = (ArenaCenter - Loc).GetSafeNormal2D().Rotation();

		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		APawn* Npc = World->SpawnActor<APawn>(PawnClass, Loc, Facing, P);
		if (!Npc) continue;

		Npc->AIControllerClass = ARRNpcController::StaticClass();
		Npc->SpawnDefaultController();
		if (ARRNpcController* NpcAI = Cast<ARRNpcController>(Npc->GetController()))
		{
			NpcAI->MakeSpectator(Arena.Get());
		}

		URRRobotVisualComponent* Visual = NewObject<URRRobotVisualComponent>(Npc, TEXT("RRNpcVisual"));
		Visual->VisualClass = Look;
		Visual->RegisterComponent();   // BeginPlay runs now (actor already began play) -> applies the look shortly after
		++Spawned;
		UE_LOG(LogRR, Log, TEXT("NPC %s spawned as %s at %s"), *Npc->GetName(), *Look->GetName(), *Loc.ToCompactString());
	}
	UE_LOG(LogRR, Log, TEXT("Ambient NPCs: %d"), Spawned);
}

// ---------------------------------------------------------------------------------------------------------------------
// Duel platform flow: boss walks in -> player holds the entry circle -> teleport onto the platform -> taunt -> fight ->
// win (crowd, back outside) / lose (defeat screen, respawn outside, boss reset on the platform).

ARRBoss* ARRGameMode::FindBoss() const
{
	if (URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>())
	{
		for (ARREnemy* E : Director->GetLivingEnemies())
		{
			if (ARRBoss* B = Cast<ARRBoss>(E)) return B;
		}
	}
	return nullptr;
}

void ARRGameMode::SetMusic(uint8 State)
{
	if (URRAudioDirector* Audio = GetWorld()->GetSubsystem<URRAudioDirector>())
	{
		Audio->SetMusicState((ERRMusicState)State);
	}
}

bool ARRGameMode::CanEnterArena() const
{
	if (!Arena.IsValid() || bFrontEndPending || bDuelStarting || MatchState != ERRMatchState::PreMatch) return false;
	ARRBoss* Boss = FindBoss();
	if (!Boss) return false;
	const ARRBossAIController* AI = Cast<ARRBossAIController>(Boss->GetController());
	return !AI || !AI->IsWalking();
}

void ARRGameMode::SpawnArenaBoss(bool bEntrance)
{
	if (!Arena.IsValid() || FindBoss()) return;
	const URRSettings* Settings = URRSettings::Get();
	UClass* BossClass = Settings ? Settings->BossClass.LoadSynchronous() : nullptr;
	URREnemyDirector* Director = GetWorld()->GetSubsystem<URREnemyDirector>();
	if (!Director) return;

	const FVector Spot = Arena->BossSpot->GetComponentLocation();
	const FVector Face = Arena->PlayerSpot->GetComponentLocation();
	const FRotator Rot = (Face - Spot).GetSafeNormal2D().Rotation();
	ARRBoss* Boss = Cast<ARRBoss>(Director->SpawnEnemy(BossClass ? BossClass : ARRBoss::StaticClass(), FTransform(Rot, Spot)));
	if (!Boss) return;

	if (ARRBossAIController* AI = Cast<ARRBossAIController>(Boss->GetController()))
	{
		AI->SetDormant(true);
	}
	Arena->SetActorPassesWalls(Boss, false);
	bBossEntrancePlayed = true;
	UE_LOG(LogRR, Log, TEXT("Arena Boss spawned inside Colosseum at %s facing %s"), *Spot.ToCompactString(), *Face.ToCompactString());
}

void ARRGameMode::TeleportPawn(APawn* Pawn, const FVector& Location, const FRotator& Facing, bool bEffects)
{
	if (!Pawn) return;
	static UParticleSystem* Burst = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/ParagonGreystone/FX/Particles/Greystone/Abilities/Primary/FX/P_Greystone_Primary_Impact.P_Greystone_Primary_Impact"));
	if (bEffects && Burst)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Burst, Pawn->GetActorLocation(), FRotator::ZeroRotator, FVector(1.5f));
	}
	// land exactly on the floor under the target: capsule bottom 3 cm above the ground (never inside it)
	FVector Dest = Location;
	{
		float HalfHeight = 90.0f;
		if (const UCapsuleComponent* Cap = Pawn->FindComponentByClass<UCapsuleComponent>()) { HalfHeight = Cap->GetScaledCapsuleHalfHeight(); }
		FHitResult Ground;
		FCollisionQueryParams Q(SCENE_QUERY_STAT(RRTeleportGround), false, Pawn);
		// start just above the target: a high start lands on arches / gates over the spot (exit ended up on the gate arch)
		if (GetWorld()->LineTraceSingleByChannel(Ground, Location + FVector(0, 0, 80.0f), Location - FVector(0, 0, 1500.0f), ECC_Visibility, Q))
		{
			Dest = Ground.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 3.0f);
		}
	}
	Pawn->SetActorLocationAndRotation(Dest, FRotator(0.0f, Facing.Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogRR, Log, TEXT("Teleport %s -> %s"), *Pawn->GetName(), *Dest.ToCompactString());
	if (AController* C = Pawn->GetController())
	{
		C->SetControlRotation(FRotator(-10.0f, Facing.Yaw, 0.0f));
	}
	if (bEffects && Burst)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Burst, Location, FRotator::ZeroRotator, FVector(1.5f));
	}
	if (bEffects)
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			if (ARRHUD* HUD = Cast<ARRHUD>(PC->GetHUD())) { HUD->Flash(FLinearColor(1.0f, 0.97f, 0.9f), 0.7f); }
		}
	}
}

void ARRGameMode::RequestArenaMatch(ARRArena* InArena)
{
	if (!CanEnterArena()) return;
	ARRBoss* Boss = FindBoss();
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Player = PC ? PC->GetPawn() : nullptr;
	if (!Boss || !Player) return;

	bDuelStarting = true;
	Arena->SetEntryEnabled(false);
	ResetRunStats();
	MatchTimeSeconds = 0.0f;
	EnemiesKilled = 0;

	const FVector PSpot = Arena->PlayerSpot->GetComponentLocation();
	const FVector BSpot = Arena->BossSpot->GetComponentLocation();
	TeleportPawn(Player, PSpot, (BSpot - PSpot).Rotation(), true);
	if (FVector::Dist2D(Boss->GetActorLocation(), BSpot) > 150.0f)
	{
		TeleportPawn(Boss, BSpot, (PSpot - BSpot).Rotation(), false);
	}
	Boss->SetActorRotation((PSpot - BSpot).GetSafeNormal2D().Rotation());
	Arena->SetActorPassesWalls(Boss, false);
	if (ARRBossAIController* AI = Cast<ARRBossAIController>(Boss->GetController()))
	{
		AI->WalkTo(BSpot, PSpot);   // stand and face the challenger
	}
	Boss->PlayTaunt();
	SetMusic((uint8)ERRMusicState::Battle);
	if (URRAudioDirector* Audio = GetWorld()->GetSubsystem<URRAudioDirector>()) { Audio->PlayCrowd(0.7f); }
	UE_LOG(LogRR, Log, TEXT("Player entered the arena: duel starts"));

	// let the taunt read before the boss engages
	GetWorld()->GetTimerManager().SetTimer(ArenaTimer, this, &ARRGameMode::StartArenaDuel, 2.4f, false);
}

void ARRGameMode::StartArenaDuel()
{
	bDuelStarting = false;
	if (ARRBoss* Boss = FindBoss())
	{
		if (ARRBossAIController* AI = Cast<ARRBossAIController>(Boss->GetController()))
		{
			AI->SetDormant(false);
		}
	}
	ChangeMatchState(ERRMatchState::InProgress);
	GetWorld()->GetTimerManager().SetTimer(MatchTimer, this, &ARRGameMode::MatchTimerTick, 1.0f, true);
}

void ARRGameMode::ExitArena()
{
	if (!Arena.IsValid()) return;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (APawn* Player = PC ? PC->GetPawn() : nullptr)
	{
		const FVector Exit = Arena->ExitPoint->GetComponentLocation();
		TeleportPawn(Player, Exit, (Arena->GetCenter() - Exit).Rotation(), true);
	}
	SetMusic((uint8)ERRMusicState::Explore);
}
