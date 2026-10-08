// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRBoss.h"
#include "RRGame.h"
#include "AAAAttributeComponent.h"
#include "AAAGameplayTags.h"
#include "RRSettings.h"
#include "AAAAWeaponActor.h"
#include "AbilitySystemGlobals.h"
#include "DataAssets/AAAWeaponSettings.h"
#include "Net/UnrealNetwork.h"
#include "AAACombatEvents.h"
#include "AAACombatComponent.h"
#include "AAAHitFeedbackSubsystem.h"
#include "AAAAttributeSet.h"
#include "AAAAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "RRBossAIController.h"

FRRBossDeathblowNative& FRRBossEvents::OnDeathblow()
{
	static FRRBossDeathblowNative Event;
	return Event;
}

FRRBossPhaseChangedNative& FRRBossEvents::OnPhaseChanged()
{
	static FRRBossPhaseChangedNative Event;
	return Event;
}

FRRBossPerilousWarningNative& FRRBossEvents::OnPerilousWarning()
{
	static FRRBossPerilousWarningNative Event;
	return Event;
}

FRRBossMikiriNative& FRRBossEvents::OnMikiri()
{
	static FRRBossMikiriNative Event;
	return Event;
}

FRRBossSpecialBeginNative& FRRBossEvents::OnSpecialAttackBegin()
{
	static FRRBossSpecialBeginNative Event;
	return Event;
}

FRRBossSpecialAttackNative& FRRBossEvents::OnSpecialAttackContact()
{
	static FRRBossSpecialAttackNative Event;
	return Event;
}

ARRBoss::ARRBoss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = ARRBossAIController::StaticClass();
	
	BossName = FText::FromString(TEXT("THE GREYSTONE"));
	Tuning.MaxPoise = 120.0f;
	Tuning.MaxHealth = 260.0f;

	// Sensible defaults for Greystone from SPEC_BOSS
	auto AddAttack = [&](FName Id, const TCHAR* AnimName, ERRBossAttackKind Kind, int32 MinPhase)
	{
		FRRBossAttack Attack;
		Attack.Id = Id;
		FString Path = FString::Printf(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/%s.%s"), AnimName, AnimName);
		Attack.Sequence = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(Path));
		Attack.Kind = Kind;
		Attack.MinPhase = MinPhase;
		Attacks.Add(Attack);
	};

	AddAttack(TEXT("Attack_A_Fast"), TEXT("Attack_A_Fast"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_A_Med"), TEXT("Attack_A_Med"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_B_Fast"), TEXT("Attack_B_Fast"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_B_Med"), TEXT("Attack_B_Med"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_C_Fast"), TEXT("Attack_C_Fast"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_C_Med"), TEXT("Attack_C_Med"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_D_Med"), TEXT("Attack_D_Med"), ERRBossAttackKind::Normal, 1);
	AddAttack(TEXT("Attack_D_Slow"), TEXT("Attack_D_Slow"), ERRBossAttackKind::PerilousSweep, 1);
	AddAttack(TEXT("Attack_A_Slow"), TEXT("Attack_A_Slow"), ERRBossAttackKind::PerilousThrust, 1);
	AddAttack(TEXT("Ability_E"), TEXT("Ability_E"), ERRBossAttackKind::PerilousSpecial, 2);
	AddAttack(TEXT("Ability_Q"), TEXT("Ability_Q"), ERRBossAttackKind::PerilousSpecial, 2);
	AddAttack(TEXT("Attack_RMB"), TEXT("Attack_RMB"), ERRBossAttackKind::Normal, 2);
}

void ARRBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARRBoss, SegmentsRemaining);
	DOREPLIFETIME(ARRBoss, Phase);
	DOREPLIFETIME(ARRBoss, bDeathblowReady);
}

void ARRBoss::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SegmentsRemaining = HealthSegments;
		Phase = 1;
		
		if (ASC)
		{
			FGameplayTag BossSegmentedTag = FGameplayTag::RequestGameplayTag(FName("State.Boss.Segmented"), false);
			if (BossSegmentedTag.IsValid())
			{
				ASC->AddLooseGameplayTag(BossSegmentedTag);
			}

			FGameplayTag BossTag = FGameplayTag::RequestGameplayTag(FName("Character.Boss"), false);
			if (BossTag.IsValid())
			{
				ASC->AddLooseGameplayTag(BossTag);
			}
		}

		HitConfirmedHandle = FAAACombatEvents::OnHitConfirmed().AddUObject(this, &ARRBoss::OnHitConfirmedHandler);
		StaggeredHandle = FAAACombatEvents::OnStaggered().AddUObject(this, &ARRBoss::OnStaggeredHandler);
		
		GetWorldTimerManager().SetTimer(PostureRegenTimer, this, &ARRBoss::RegenPostureTick, 0.1f, true);

		if (ASC)
		{
			FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.Hit"), false);
			if (HitTag.IsValid())
			{
				ASC->GenericGameplayEventCallbacks.FindOrAdd(HitTag).AddUObject(this, &ARRBoss::OnWeaponHit);
			}
		}
	}
}

void ARRBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		FAAACombatEvents::OnHitConfirmed().Remove(HitConfirmedHandle);
		FAAACombatEvents::OnStaggered().Remove(StaggeredHandle);
		GetWorldTimerManager().ClearTimer(PostureRegenTimer);
		GetWorldTimerManager().ClearTimer(TraceStartHandle);
		GetWorldTimerManager().ClearTimer(TraceEndHandle);
		GetWorldTimerManager().ClearTimer(AttackRecoverHandle);
		GetWorldTimerManager().ClearTimer(PerilousWarningHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ARRBoss::OnRep_SegmentsRemaining()
{
	// HUD can listen to phase changes or deathblows, but replication gives current state
}

void ARRBoss::OnRep_Phase()
{
	// Trigger HUD or effects
}

void ARRBoss::RegenPostureTick()
{
	if (!ASC || !AttributeSet) return;
	
	FGameplayTag StaggeredTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false);
	if (StaggeredTag.IsValid() && ASC->HasMatchingGameplayTag(StaggeredTag))
	{
		return;
	}

	float CurrentPoise = AttributeSet->GetPoise();
	float MaxPoise = AttributeSet->GetMaxPoise();
	
	if (CurrentPoise < MaxPoise)
	{
		// Native poise regen kicks in from UAAACombatComponent, but we can override or rely on it.
		// Boss spec says: Poise regen starts PostureRegenDelay after last poise damage.
		// For simplicity, rely on the global setting handled in UAAACombatComponent, 
		// but since the boss requires a specific delay, we could do it here if needed.
	}
}

void ARRBoss::OnHitConfirmedHandler(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	if (Victim != this) return;

	APawn* AttackerPawn = Cast<APawn>(Attacker);
	UE_LOG(LogRR, Verbose, TEXT("Boss hit by %s (player=%d, deathblowReady=%d)"), *GetNameSafe(Attacker), AttackerPawn ? (int32)AttackerPawn->IsPlayerControlled() : -1, (int32)bDeathblowReady);
	if (AttackerPawn && AttackerPawn->IsPlayerControlled())
	{
		bool bIsStaggered = false;
		if (ASC)
		{
			FGameplayTag StaggeredTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false);
			bIsStaggered = StaggeredTag.IsValid() && ASC->HasMatchingGameplayTag(StaggeredTag);
		}

		if (bIsStaggered || bDeathblowReady)
		{
			ExecuteDeathblow(Attacker);
		}
	}
}

void ARRBoss::OnStaggeredHandler(AActor* Victim)
{
	if (Victim == this)
	{
		UE_LOG(LogRR, Log, TEXT("Boss posture/segment broken: deathblow ready"));
		bDeathblowReady = true;
	}
}

void ARRBoss::OnDeathHandler()
{
	bDeathblowReady = false;
}

void ARRBoss::ExecuteDeathblow(AActor* Attacker)
{
	UE_LOG(LogRR, Log, TEXT("Boss DEATHBLOW (segments %d -> %d)"), SegmentsRemaining, SegmentsRemaining - 1);
	if (SegmentsRemaining <= 0) return;

	SegmentsRemaining--;
	bDeathblowReady = false;

	if (ASC)
	{
		FGameplayTag StaggeredTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false);
		if (StaggeredTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(StaggeredTag);
		}
		
		ASC->SetNumericAttributeBase(UAAAAttributeSet::GetPoiseAttribute(), AttributeSet->GetMaxPoise());
	}

	if (CombatComponent)
	{
		CombatComponent->PlayHitReaction(Attacker, true); // Play heavy hit reaction
	}

	if (SegmentsRemaining > 0)
	{
		if (ASC)
		{
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), SegmentMaxHealth);
		}
		
		Phase++;
		Tuning.AttackCooldownMin *= 0.7f;
		Tuning.AttackCooldownMax *= 0.7f;
		Tuning.ChaseSpeed *= 1.15f;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = Tuning.ChaseSpeed;
		}

		MulticastPhaseChangedPresentation(Phase);
	}
	else
	{
		if (ASC)
		{
			// Last segment: stop being "segmented", then let the normal death path run (HealthChanged -> OnDeath -> director).
			ASC->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Boss_Segmented);
			ASC->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Boss_Perilous);
			ASC->AddLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead);
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), 1.0f);
		}
		if (AttributeComponent)
		{
			AttributeComponent->Kill();
		}
	}

	MulticastDeathblowPresentation();
}

void ARRBoss::MulticastDeathblowPresentation_Implementation()
{
	if (UAAAHitFeedbackSubsystem* HitFeedback = GetWorld()->GetSubsystem<UAAAHitFeedbackSubsystem>())
	{
		// Using the existing hitstop logic inside HitFeedback would be ideal.
		// However, hitstop is usually applied automatically on OnHitConfirmed.
		// We can broadcast the static event for the HUD.
	}
	
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->ClientStartCameraShake(UAAACombatCameraShake::StaticClass(), 3.0f);
	}
	
	FRRBossEvents::OnDeathblow().Broadcast(this);
}

void ARRBoss::MulticastPhaseChangedPresentation_Implementation(int32 NewPhase)
{
	FRRBossEvents::OnPhaseChanged().Broadcast(this, NewPhase);
}

float ARRBoss::GetSegmentFraction() const
{
	if (ASC && AttributeSet)
	{
		float Health = ASC->GetNumericAttribute(UAAAAttributeSet::GetHealthAttribute());
		return Health / FMath::Max(1.0f, SegmentMaxHealth);
	}
	return 1.0f;
}

float ARRBoss::GetPostureFraction() const
{
	if (ASC && AttributeSet)
	{
		float Poise = ASC->GetNumericAttribute(UAAAAttributeSet::GetPoiseAttribute());
		float MaxP = ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxPoiseAttribute());
		return FMath::Clamp(1.0f - (Poise / FMath::Max(1.0f, MaxP)), 0.0f, 1.0f);
	}
	return 0.0f;
}

bool ARRBoss::IsPostureBroken() const
{
	if (ASC)
	{
		FGameplayTag StaggeredTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false);
		return StaggeredTag.IsValid() && ASC->HasMatchingGameplayTag(StaggeredTag);
	}
	return false;
}

void ARRBoss::OnWeaponHit(const FGameplayEventData* Payload)
{
	if (!Payload || !Payload->Target || !ASC) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload->Target);
	if (!TargetASC) return;
	
	AActor* TargetActor = const_cast<AActor*>(Payload->Target.Get());

	if (CurrentAttackKind == ERRBossAttackKind::PerilousSpecial)
	{
		// Cinematic hook
		FRRBossEvents::OnSpecialAttackContact().Broadcast(this, TargetActor);
		// For now it behaves as a normal unblockable heavy hit
	}
	else if (CurrentAttackKind == ERRBossAttackKind::PerilousThrust)
	{
		FGameplayTag InvTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Invulnerable"), false);
		if (InvTag.IsValid() && TargetASC->HasMatchingGameplayTag(InvTag))
		{
			FVector ToBoss = (GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
			FVector TargetFacing = TargetActor->GetActorForwardVector().GetSafeNormal2D();
			if (FVector::DotProduct(TargetFacing, ToBoss) > 0.3f)
			{
				// Mikiri!
				if (ASC)
				{
					float CurrentPoise = AttributeSet ? AttributeSet->GetPoise() : 120.0f;
					ASC->SetNumericAttributeBase(UAAAAttributeSet::GetPoiseAttribute(), FMath::Max(0.0f, CurrentPoise - 70.0f));
					if (CombatComponent)
					{
						CombatComponent->ApplyTimedState(FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false), 1.5f);
					}
				}
				FRRBossEvents::OnMikiri().Broadcast(this, TargetActor);
				return; // Target avoids damage
			}
		}
	}
	
	UClass* DamageEffectClass = StaticLoadClass(UGameplayEffect::StaticClass(), nullptr, TEXT("/Script/AAAAbility.AAAGE_Damage"));
	if (!DamageEffectClass) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(this, this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		float BaseDmg = 10.0f; 
		if (CombatComponent && CombatComponent->GetCurrentWeaponSettings()) 
		{
			BaseDmg = CombatComponent->GetCurrentWeaponSettings()->Combat.BaseDamage;
		}

		FGameplayTag DataDamage = FGameplayTag::RequestGameplayTag(FName("Data.Damage"), false);
		FGameplayTag DataPoise = FGameplayTag::RequestGameplayTag(FName("Data.PoiseDamage"), false);
		
		if (DataDamage.IsValid()) SpecHandle.Data->SetSetByCallerMagnitude(DataDamage, BaseDmg * CurrentAttackDamageMult);
		if (DataPoise.IsValid()) SpecHandle.Data->SetSetByCallerMagnitude(DataPoise, CurrentAttackPoiseDamage);
		
		ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void ARRBoss::PlayAttackMontage(const FRRBossAttack& Attack)
{
	if (!CombatComponent) return;
	
	UAnimInstance* AnimInst = CombatComponent->FindAnimInstance();
	if (!AnimInst) return;
	
	UAnimMontage* MontageToPlay = Attack.Montage.LoadSynchronous();
	if (!MontageToPlay && Attack.Sequence.LoadSynchronous())
	{
		MontageToPlay = AnimInst->PlaySlotAnimationAsDynamicMontage(Attack.Sequence.Get(), FName("DefaultSlot"), 0.15f, 0.2f, Attack.PlayRate);
	}
	else if (MontageToPlay)
	{
		AnimInst->Montage_Play(MontageToPlay, Attack.PlayRate);
	}
	
	if (!MontageToPlay) return;

	AAAAWeaponActor* Weapon = CombatComponent->GetCurrentWeapon();
	if (!Weapon) return;

	float MontageLen = MontageToPlay->GetPlayLength() / FMath::Max(0.01f, Attack.PlayRate);
	float StartTime = MontageLen * Attack.HitWindowStart;
	if (Attack.bCinematic)
	{
		FRRBossEvents::OnSpecialAttackBegin().Broadcast(this, GetCurrentTarget(), StartTime);
	}
	float EndTime = MontageLen * Attack.HitWindowEnd;
	
	CurrentAttackDamageMult = Attack.DamageMultiplier;
	CurrentAttackPoiseDamage = Attack.PoiseDamage;
	CurrentAttackKind = Attack.Kind;

	if (ASC)
	{
		FGameplayTag PerilousTag = FGameplayTag::RequestGameplayTag(FName("State.Boss.Perilous"), false);
		if (CurrentAttackKind != ERRBossAttackKind::Normal)
		{
			if (PerilousTag.IsValid()) ASC->AddLooseGameplayTag(PerilousTag);
			
			float WarnTime = FMath::Max(0.01f, StartTime - 0.6f);
			GetWorldTimerManager().SetTimer(PerilousWarningHandle, [this, Kind = CurrentAttackKind, Secs = (StartTime - WarnTime)]()
			{
				FRRBossEvents::OnPerilousWarning().Broadcast(this, Kind, Secs);
			}, WarnTime, false);
		}
		else
		{
			if (PerilousTag.IsValid()) ASC->RemoveLooseGameplayTag(PerilousTag);
		}
	}

	TWeakObjectPtr<AAAAWeaponActor> WeakWeapon(Weapon);
	GetWorldTimerManager().SetTimer(TraceStartHandle, [WeakWeapon]() { if (AAAAWeaponActor* W = WeakWeapon.Get()) W->StartWeaponTrace(); }, StartTime, false);
	GetWorldTimerManager().SetTimer(TraceEndHandle, [WeakWeapon]() { if (AAAAWeaponActor* W = WeakWeapon.Get()) W->StopWeaponTrace(); }, EndTime, false);
	
	GetWorldTimerManager().SetTimer(AttackRecoverHandle, [this]()
	{
		if (ASC)
		{
			FGameplayTag PerilousTag = FGameplayTag::RequestGameplayTag(FName("State.Boss.Perilous"), false);
			if (PerilousTag.IsValid()) ASC->RemoveLooseGameplayTag(PerilousTag);
		}
	}, MontageLen + Attack.RecoverySeconds, false);
}

#if !UE_BUILD_SHIPPING
#include "RREnemyDirector.h"

static ARRBoss* GetDebugBoss(UWorld* World)
{
	if (URREnemyDirector* Director = World->GetSubsystem<URREnemyDirector>())
	{
		for (ARREnemy* Enemy : Director->GetLivingEnemies())
		{
			if (ARRBoss* Boss = Cast<ARRBoss>(Enemy))
			{
				return Boss;
			}
		}
	}
	return nullptr;
}

static void RRBossSpawnCommand(const TArray<FString>& Args, UWorld* World)
{
	if (!World) return;
	URREnemyDirector* Director = World->GetSubsystem<URREnemyDirector>();
	APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Director || !Player) return;
	
	const URRSettings* Settings = URRSettings::Get();
	UClass* Class = Settings ? Settings->BossClass.LoadSynchronous() : nullptr;
	FVector Dir = Player->GetActorForwardVector();
	FVector Loc = Player->GetActorLocation() + Dir * 1400.0f;
	Director->SpawnEnemy(Class ? Class : ARRBoss::StaticClass(), FTransform((-Dir).Rotation(), Loc));
}

static void RRBossHurtCommand(const TArray<FString>& Args, UWorld* World)
{
	if (ARRBoss* Boss = GetDebugBoss(World))
	{
		float Frac = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.5f;
		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent())
		{
			float Cur = ASC->GetNumericAttribute(UAAAAttributeSet::GetHealthAttribute());
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetHealthAttribute(), Cur - (Boss->SegmentMaxHealth * Frac));
		}
	}
}

static void RRBossBreakCommand(const TArray<FString>& Args, UWorld* World)
{
	if (ARRBoss* Boss = GetDebugBoss(World))
	{
		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(UAAAAttributeSet::GetPoiseAttribute(), 0.0f);
			if (UAAACombatComponent* CC = Boss->FindComponentByClass<UAAACombatComponent>())
			{
				CC->ApplyTimedState(FGameplayTag::RequestGameplayTag(FName("State.Combat.Staggered"), false), Boss->StaggerSeconds);
			}
		}
	}
}

static void RRBossPhase2Command(const TArray<FString>& Args, UWorld* World)
{
	if (ARRBoss* Boss = GetDebugBoss(World))
	{
		Boss->ExecuteDeathblow(UGameplayStatics::GetPlayerPawn(World, 0));
	}
}

static void RRBossAttackCommand(const TArray<FString>& Args, UWorld* World)
{
	if (ARRBoss* Boss = GetDebugBoss(World))
	{
		if (Args.Num() > 0)
		{
			FName Id = FName(*Args[0]);
			for (const FRRBossAttack& Att : Boss->Attacks)
			{
				if (Att.Id == Id)
				{
					Boss->PlayAttackMontage(Att);
					break;
				}
			}
		}
	}
}

static void RRBossPerilousCommand(const TArray<FString>& Args, UWorld* World)
{
	if (ARRBoss* Boss = GetDebugBoss(World))
	{
		for (const FRRBossAttack& Att : Boss->Attacks)
		{
			if (Att.Kind == ERRBossAttackKind::PerilousThrust)
			{
				Boss->PlayAttackMontage(Att);
				break;
			}
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs GRRBossSpawnCmd(TEXT("rr.boss.spawn"), TEXT("rr.boss.spawn - spawn a boss in front of the player"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossSpawnCommand));
static FAutoConsoleCommandWithWorldAndArgs GRRBossHurtCmd(TEXT("rr.boss.hurt"), TEXT("rr.boss.hurt [fraction] - damage the boss's current health segment"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossHurtCommand));
static FAutoConsoleCommandWithWorldAndArgs GRRBossBreakCmd(TEXT("rr.boss.break"), TEXT("rr.boss.break - break the boss's posture"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossBreakCommand));
static FAutoConsoleCommandWithWorldAndArgs GRRBossPhase2Cmd(TEXT("rr.boss.phase2"), TEXT("rr.boss.phase2 - force phase 2"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossPhase2Command));
static FAutoConsoleCommandWithWorldAndArgs GRRBossAttackCmd(TEXT("rr.boss.attack"), TEXT("rr.boss.attack <id> - force an attack"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossAttackCommand));
static FAutoConsoleCommandWithWorldAndArgs GRRBossPerilousCmd(TEXT("rr.boss.perilous"), TEXT("rr.boss.perilous - force perilous thrust"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RRBossPerilousCommand));
#endif
