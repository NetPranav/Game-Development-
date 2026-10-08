// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RREnemyDirector.h"
#include "RREnemy.h"
#include "RRGame.h"
#include "RRSettings.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "HAL/IConsoleManager.h"
#include "Containers/Ticker.h"
#include "AAACombatComponent.h"
#include "AAAAWeaponActor.h"

bool URREnemyDirector::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	// Not needed on pure clients: AI and the match flow are server / standalone only.
	return World && World->GetNetMode() != NM_Client && Super::ShouldCreateSubsystem(Outer);
}

bool URREnemyDirector::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void URREnemyDirector::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	for (TActorIterator<ARREnemy> It(&InWorld); It; ++It)
	{
		Register(*It);
	}
	ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &URREnemyDirector::HandleActorSpawned));
}

void URREnemyDirector::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
	}
	Enemies.Reset();
	TokenHolders.Reset();
	Super::Deinitialize();
}

void URREnemyDirector::HandleActorSpawned(AActor* Actor)
{
	if (ARREnemy* Enemy = Cast<ARREnemy>(Actor))
	{
		Register(Enemy);
	}
}

void URREnemyDirector::Register(ARREnemy* Enemy)
{
	if (!IsValid(Enemy) || Enemies.Contains(Enemy))
	{
		return;
	}
	Enemies.Add(Enemy);
	OnEnemySpawned.Broadcast(Enemy);
}

void URREnemyDirector::Unregister(ARREnemy* Enemy)
{
	Enemies.Remove(Enemy);
}

ARREnemy* URREnemyDirector::SpawnEnemy(TSubclassOf<ARREnemy> EnemyClass, const FTransform& Transform)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (!EnemyClass)
	{
		EnemyClass = URRSettings::Get() ? URRSettings::Get()->DefaultEnemyClass.LoadSynchronous() : nullptr;
		if (!EnemyClass)
		{
			EnemyClass = ARREnemy::StaticClass();
		}
	}

	// Snap to the ground so enemies never spawn floating or buried.
	FTransform SpawnTransform = Transform;
	FHitResult Hit;
	const FVector Start = Transform.GetLocation() + FVector(0.f, 0.f, 600.f);
	const FVector End = Transform.GetLocation() - FVector(0.f, 0.f, 2000.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RREnemySpawnGround), false);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		const ACharacter* CDO = EnemyClass->GetDefaultObject<ACharacter>();
		const float HalfHeight = CDO && CDO->GetCapsuleComponent() ? CDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;
		SpawnTransform.SetLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 5.f));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ARREnemy* Enemy = World->SpawnActor<ARREnemy>(EnemyClass, SpawnTransform, SpawnParams);
	if (!Enemy)
	{
		UE_LOG(LogRR, Warning, TEXT("URREnemyDirector::SpawnEnemy failed for class %s"), *GetNameSafe(EnemyClass));
	}
	// Registration happens through HandleActorSpawned.
	return Enemy;
}

TArray<ARREnemy*> URREnemyDirector::GetLivingEnemies() const
{
	TArray<ARREnemy*> Result;
	for (const TWeakObjectPtr<ARREnemy>& Weak : Enemies)
	{
		if (ARREnemy* Enemy = Weak.Get())
		{
			if (Enemy->IsAlive())
			{
				Result.Add(Enemy);
			}
		}
	}
	return Result;
}

int32 URREnemyDirector::GetLivingEnemyCount() const
{
	return GetLivingEnemies().Num();
}

void URREnemyDirector::NotifyEnemyDied(ARREnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	if (AController* Controller = Enemy->GetController())
	{
		ReleaseAttackToken(Controller);
	}

	// Count survivors explicitly excluding the one that just died (its alive-flag may lag behind this call).
	int32 Remaining = 0;
	for (const TWeakObjectPtr<ARREnemy>& Weak : Enemies)
	{
		const ARREnemy* Other = Weak.Get();
		if (Other && Other != Enemy && Other->IsAlive())
		{
			++Remaining;
		}
	}

	OnEnemyDied.Broadcast(Enemy, Remaining);
	if (Remaining == 0)
	{
		OnAllEnemiesDefeated.Broadcast();
	}
}

void URREnemyDirector::PurgeTokens()
{
	TokenHolders.RemoveAll([](const TWeakObjectPtr<AController>& Holder)
	{
		const AController* C = Holder.Get();
		if (!C)
		{
			return true;
		}
		const ARREnemy* Enemy = Cast<ARREnemy>(C->GetPawn());
		return !Enemy || !Enemy->IsAlive();
	});
}

bool URREnemyDirector::TryAcquireAttackToken(AController* Requester)
{
	if (!Requester)
	{
		return false;
	}
	PurgeTokens();
	if (TokenHolders.Contains(Requester))
	{
		return true;
	}
	const URRSettings* Settings = URRSettings::Get();
	const int32 MaxHolders = FMath::Max(1, Settings ? Settings->MaxSimultaneousAttackers : 2);
	if (TokenHolders.Num() < MaxHolders)
	{
		TokenHolders.Add(Requester);
		return true;
	}
	return false;
}

void URREnemyDirector::ReleaseAttackToken(AController* Requester)
{
	TokenHolders.Remove(Requester);
}

#if !UE_BUILD_SHIPPING
static void RRSpawnEnemiesCommand(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		return;
	}
	URREnemyDirector* Director = World->GetSubsystem<URREnemyDirector>();
	APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Director || !Player)
	{
		UE_LOG(LogRR, Warning, TEXT("rr.enemy.spawn needs a running game with a player pawn"));
		return;
	}
	const int32 Count = Args.Num() > 0 ? FMath::Clamp(FCString::Atoi(*Args[0]), 1, 20) : 1;
	const float Distance = Args.Num() > 1 ? FMath::Max(200.f, FCString::Atof(*Args[1])) : 900.f;
	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = (Count == 1) ? 0.f : (360.f / Count) * i;
		const FVector Dir = Player->GetActorForwardVector().RotateAngleAxis(Angle, FVector::UpVector);
		const FVector Loc = Player->GetActorLocation() + Dir * Distance;
		const FRotator Facing = (-Dir).Rotation();
		Director->SpawnEnemy(nullptr, FTransform(Facing, Loc));
	}
}

static void RRKillEnemiesCommand(const TArray<FString>& Args, UWorld* World)
{
	if (URREnemyDirector* Director = World ? World->GetSubsystem<URREnemyDirector>() : nullptr)
	{
		for (ARREnemy* Enemy : Director->GetLivingEnemies())
		{
			Enemy->Destroy();
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs GRRSpawnEnemyCmd(
	TEXT("rr.enemy.spawn"), TEXT("rr.enemy.spawn [count] [distance] - spawn enemies around the player"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRSpawnEnemiesCommand));

static FAutoConsoleCommandWithWorldAndArgs GRRKillEnemyCmd(
	TEXT("rr.enemy.killall"), TEXT("rr.enemy.killall - destroy all living enemies (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRKillEnemiesCommand));

static FTSTicker::FDelegateHandle GRRAutoplayHandle;

/** Test bot: makes the local player face the nearest living enemy and attack when it is in reach. */
static void RRAutoplayCommand(const TArray<FString>& Args, UWorld* World)
{
	if (GRRAutoplayHandle.IsValid())
	{
		FTSTicker::RemoveTicker(GRRAutoplayHandle);
		GRRAutoplayHandle.Reset();
		UE_LOG(LogRR, Log, TEXT("rr.test.autoplay: off"));
		return;
	}

	TWeakObjectPtr<UWorld> WeakWorld(World);
	UE_LOG(LogRR, Log, TEXT("rr.test.autoplay: on"));
	GRRAutoplayHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld](float) -> bool
	{
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			GRRAutoplayHandle.Reset();
			return false;
		}
		APawn* Player = UGameplayStatics::GetPlayerPawn(W, 0);
		URREnemyDirector* Director = W->GetSubsystem<URREnemyDirector>();
		if (!Player || !Director)
		{
			return true;
		}
		ARREnemy* Best = nullptr;
		float BestDist = TNumericLimits<float>::Max();
		for (ARREnemy* Enemy : Director->GetLivingEnemies())
		{
			const float D = FVector::Dist(Enemy->GetActorLocation(), Player->GetActorLocation());
			if (D < BestDist)
			{
				BestDist = D;
				Best = Enemy;
			}
		}
		if (Best && BestDist < 320.f)
		{
			FRotator Face = (Best->GetActorLocation() - Player->GetActorLocation()).Rotation();
			Face.Pitch = 0.f;
			Face.Roll = 0.f;
			Player->SetActorRotation(Face);
			if (UAAACombatComponent* Combat = Player->FindComponentByClass<UAAACombatComponent>())
			{
				static int32 Counter = 0;
				Combat->RequestAttack((++Counter % 4 == 0) ? EAAAAttackType::Heavy : EAAAAttackType::Light);
			}
		}
		return true;
	}), 0.25f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRAutoplayCmd(
	TEXT("rr.test.autoplay"), TEXT("rr.test.autoplay - toggle a bot that attacks the nearest enemy (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRAutoplayCommand));

/** rr.test.shot [delaySeconds] [quitAfter=1]: takes a high-res screenshot after the delay (then quits). Test helper. */
static void RRShotCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Delay = Args.Num() > 0 ? FMath::Max(0.5f, FCString::Atof(*Args[0])) : 10.f;
	const bool bQuit = Args.Num() > 1 ? FCString::Atoi(*Args[1]) != 0 : true;
	TWeakObjectPtr<UWorld> WeakWorld(World);
	auto ShotTime = MakeShared<float>(0.f);
	auto Shot = MakeShared<bool>(false);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld, Delay, bQuit, ShotTime, Shot](float Dt) -> bool
	{
		*ShotTime += Dt;
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			return false;
		}
		if (!*Shot && *ShotTime >= Delay)
		{
			*Shot = true;
			GEngine->Exec(W, TEXT("HighResShot 1"));
			UE_LOG(LogRR, Log, TEXT("rr.test.shot: HighResShot requested"));
		}
		if (*Shot && bQuit && *ShotTime >= Delay + 3.f)
		{
			FPlatformMisc::RequestExit(false);
			return false;
		}
		return !*Shot || bQuit;
	}), 0.1f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRShotCmd(
	TEXT("rr.test.shot"), TEXT("rr.test.shot [delay] [quit] - screenshot after delay seconds (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRShotCommand));

/** rr.test.draw: waits for the player's weapon data, then draws the weapon (test helper). */
static void RRDrawCommand(const TArray<FString>& Args, UWorld* World)
{
	TWeakObjectPtr<UWorld> WeakWorld(World);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld](float) -> bool
	{
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			return false;
		}
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(W, 0))
		{
			if (UAAACombatComponent* Combat = Player->FindComponentByClass<UAAACombatComponent>())
			{
				if (Combat->GetCurrentWeaponSettings())
				{
					Combat->DrawWeapon();
					return false;
				}
			}
		}
		return true;
	}), 0.25f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRDrawCmd(
	TEXT("rr.test.draw"), TEXT("rr.test.draw - draw the player's weapon once it is equipped (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRDrawCommand));

/** rr.test.swordrot <roll> <pitch> <yaw>: after the weapon is drawn, forces the hand offset rotation (test helper for tuning). */
static void RRSwordRotCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Roll = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.f;
	const float Pitch = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
	const float Yaw = Args.Num() > 2 ? FCString::Atof(*Args[2]) : 0.f;
	TWeakObjectPtr<UWorld> WeakWorld(World);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld, Roll, Pitch, Yaw](float) -> bool
	{
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			return false;
		}
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(W, 0))
		{
			if (UAAACombatComponent* Combat = Player->FindComponentByClass<UAAACombatComponent>())
			{
				if (AActor* Weapon = Combat->GetCurrentWeapon())
				{
					Weapon->SetActorRelativeRotation(FRotator(Pitch, Yaw, Roll));
				}
			}
		}
		return true;
	}), 0.2f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRSwordRotCmd(
	TEXT("rr.test.swordrot"), TEXT("rr.test.swordrot <roll> <pitch> <yaw> - live-tune the weapon hand rotation (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRSwordRotCommand));

/** rr.test.tab [delay]: presses and releases Tab on the local player controller after a delay (opens/closes the inventory). */
static void RRTabCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Delay = Args.Num() > 0 ? FMath::Max(0.1f, FCString::Atof(*Args[0])) : 6.f;
	TWeakObjectPtr<UWorld> WeakWorld(World);
	auto Elapsed = MakeShared<float>(0.f);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld, Delay, Elapsed](float Dt) -> bool
	{
		*Elapsed += Dt;
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			return false;
		}
		if (*Elapsed < Delay)
		{
			return true;
		}
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0))
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::Tab, EInputEvent::IE_Pressed, 1.0f));
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::Tab, EInputEvent::IE_Released, 0.0f));
			UE_LOG(LogRR, Log, TEXT("rr.test.tab: Tab pressed"));
		}
		return false;
	}), 0.1f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRTabCmd(
	TEXT("rr.test.tab"), TEXT("rr.test.tab [delay] - press Tab after a delay (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRTabCommand));

/** rr.test.key <KeyName> [delay]: presses and releases any key on the local player controller (test helper). */
static void RRKeyCommand(const TArray<FString>& Args, UWorld* World)
{
	if (Args.Num() < 1)
	{
		return;
	}
	const FKey Key(*Args[0]);
	const float Delay = Args.Num() > 1 ? FMath::Max(0.1f, FCString::Atof(*Args[1])) : 1.f;
	TWeakObjectPtr<UWorld> WeakWorld(World);
	auto Elapsed = MakeShared<float>(0.f);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld, Key, Delay, Elapsed](float Dt) -> bool
	{
		*Elapsed += Dt;
		UWorld* W = WeakWorld.Get();
		if (!W)
		{
			return false;
		}
		if (*Elapsed < Delay)
		{
			return true;
		}
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0))
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, EInputEvent::IE_Pressed, 1.0f));
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, EInputEvent::IE_Released, 0.0f));
			UE_LOG(LogRR, Log, TEXT("rr.test.key: %s pressed"), *Key.ToString());
		}
		return false;
	}), 0.1f);
}

static FAutoConsoleCommandWithWorldAndArgs GRRKeyCmd(
	TEXT("rr.test.key"), TEXT("rr.test.key <KeyName> [delay] - press a key after a delay (test helper)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRKeyCommand));
#endif
