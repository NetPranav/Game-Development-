// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "GameFramework/AAAAPlayerController.h"
#include "GameFramework/AAAAHUD.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AAACombatComponent.h"
#include "AAAGameplayTags.h"
#include "AAADebugLogger.h"
#include "AAAAttributeSet.h"
#include "AAACombatSettings.h"
#include "GenericTeamAgentInterface.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"

AAAAAPlayerController::AAAAAPlayerController()
{
	// Default constructor
}

void AAAAAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize local player systems, bind Enhanced Input Mapping Contexts (for UI, Pause, etc.)
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (GlobalUIMappingContext)
		{
			Subsystem->AddMappingContext(GlobalUIMappingContext, 1); // High priority for UI menus
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("SUCCESS: Global UI Mapping Context Added!"));
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("CRITICAL ERROR: Global UI Mapping Context is empty in BP_AAAPlayerController!"));
		}
	}
}

void AAAAAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("SetupInputComponent RAN on Player Controller"));

	// Bind global actions (Pause Menu, Inventory Toggle, etc.)
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleMenuAction)
		{
			EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &AAAAAPlayerController::OnToggleMenu);
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("CRITICAL ERROR: Toggle Menu Action is empty in BP_AAAPlayerController!"));
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("CRITICAL ERROR: Cast to UEnhancedInputComponent failed! InputComponent is not Enhanced Input."));
	}

	// Direct input actions for Combat, Dodge, and Weapon Draw/Sheath
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AAAAAPlayerController::ToggleLockOn);
	InputComponent->BindKey(EKeys::MiddleMouseButton, IE_Pressed, this, &AAAAAPlayerController::ToggleLockOn);
	InputComponent->BindKey(EKeys::LeftBracket, IE_Pressed, this, &AAAAAPlayerController::CycleLockOnLeft);
	InputComponent->BindKey(EKeys::RightBracket, IE_Pressed, this, &AAAAAPlayerController::CycleLockOnRight);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AAAAAPlayerController::OnDrawSheathWeapon);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AAAAAPlayerController::OnDrawSheathWeapon);
	InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AAAAAPlayerController::OnDrawSheathWeapon);
	InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AAAAAPlayerController::OnDrawSheathWeapon);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AAAAAPlayerController::OnAttackAction);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AAAAAPlayerController::OnHeavyAttackAction);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AAAAAPlayerController::OnDodgeAction);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AAAAAPlayerController::OnBlockPressed);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AAAAAPlayerController::OnBlockReleased);

	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AAAAAPlayerController::CloseMenuIfOpen).bExecuteWhenPaused = true;
}

void AAAAAPlayerController::OnDrawSheathWeapon()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->ToggleWeaponDraw();
		}
	}
}

void AAAAAPlayerController::OnAttackAction()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->RequestAttack(EAAAAttackType::Light);
		}
	}
}

void AAAAAPlayerController::OnHeavyAttackAction()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->RequestAttack(EAAAAttackType::Heavy);
		}
	}
}

void AAAAAPlayerController::OnDodgeAction()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAAAGameplayTags::Get().InputTag_Dodge);
				ASC->TryActivateAbilitiesByTag(TagContainer);
			}
		}
	}
}

void AAAAAPlayerController::OnToggleMenu()
{
	if (AAAAAHUD* GameHUD = Cast<AAAAAHUD>(GetHUD()))
	{
		bool bIsNowOpen = GameHUD->ToggleGameMenu();

		if (bIsNowOpen)
		{
			FAAADebugLogger::LogAction(TEXT("UI"), TEXT("Game Menu OPENED | Game Paused | Cursor Unlocked"), FColor::Green);
			// Menu is OPEN: Switch to Game And UI input so we can still press the close hotkey!
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways); // Trap the mouse in the game window!
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
			
			// Pause the game (stops WASD, stops the world)
			SetPause(true);
		}
		else
		{
			FAAADebugLogger::LogAction(TEXT("UI"), TEXT("Game Menu CLOSED | Game Resumed | Cursor Locked"), FColor(180, 255, 180));
			// Menu is CLOSED: Switch back to Game only, hide cursor, UNPAUSE GAME
			SetInputMode(FInputModeGameOnly());
			SetShowMouseCursor(false);
			
			SetPause(false);
		}
	}
}

void AAAAAPlayerController::CloseMenuIfOpen()
{
	if (IsPaused())
	{
		OnToggleMenu();
	}
}

void AAAAAPlayerController::OnBlockPressed()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->SetBlocking(true);
		}
	}
}

void AAAAAPlayerController::OnBlockReleased()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->SetBlocking(false);
		}
	}
}

void AAAAAPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearLock();
	Super::EndPlay(EndPlayReason);
}

void AAAAAPlayerController::OnUnPossess()
{
	ClearLock();
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UAAACombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UAAACombatComponent>())
		{
			CombatComp->SetBlocking(false);
		}
	}
	Super::OnUnPossess();
}

void AAAAAPlayerController::ClearLock()
{
	if (LockedTarget)
	{
		LockedTarget = nullptr;
		TimeWithoutLOS = 0.0f;
		TimeLookInputFlick = 0.0f;
		OnLockTargetChanged.Broadcast(nullptr);
	}
}

TArray<AActor*> AAAAAPlayerController::GetLockOnCandidates() const
{
	TArray<AActor*> Result;
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return Result;

	UWorld* World = GetWorld();
	if (!World) return Result;

	const UAAACombatSettings* Settings = UAAACombatSettings::Get();
	if (!Settings) return Result;

	float MaxRange = Settings->LockOnRange;
	float MaxAngle = Settings->LockOnMaxAngleDegrees;

	FVector CamLoc;
	FRotator CamRot;
	GetPlayerViewPoint(CamLoc, CamRot);
	FVector CamForward = CamRot.Vector();

	uint8 MyTeam = FGenericTeamId::NoTeam;
	if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(ControlledPawn))
	{
		MyTeam = TeamAgent->GetGenericTeamId().GetId();
	}
	else if (IGenericTeamAgentInterface* PCTeamAgent = Cast<IGenericTeamAgentInterface>(const_cast<AAAAAPlayerController*>(this)))
	{
		MyTeam = PCTeamAgent->GetGenericTeamId().GetId();
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ControlledPawn);

	World->OverlapMultiByObjectType(Overlaps, ControlledPawn->GetActorLocation(), FQuat::Identity, 
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), 
		FCollisionShape::MakeSphere(MaxRange), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Result.Contains(Actor)) continue;

		UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>();
		if (!ASC)
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
			{
				ASC = ASI->GetAbilitySystemComponent();
			}
		}
		if (!ASC) continue;

		if (ASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead)) continue;

		uint8 OtherTeam = FGenericTeamId::NoTeam;
		if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Actor))
		{
			OtherTeam = TeamAgent->GetGenericTeamId().GetId();
		}

		if (MyTeam != FGenericTeamId::NoTeam && OtherTeam != FGenericTeamId::NoTeam && MyTeam == OtherTeam)
		{
			continue;
		}

		FVector TargetChest = Actor->GetActorLocation();
		FVector DirToTarget = (TargetChest - CamLoc).GetSafeNormal();
		
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CamForward, DirToTarget)));
		if (AngleDegrees > MaxAngle) continue;

		FVector PawnEye;
		FRotator PawnEyeRot;
		ControlledPawn->GetActorEyesViewPoint(PawnEye, PawnEyeRot);

		FHitResult HitResult;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(ControlledPawn);
		TraceParams.AddIgnoredActor(Actor);
		bool bHit = World->LineTraceSingleByChannel(HitResult, PawnEye, TargetChest, ECC_Visibility, TraceParams);
		
		if (bHit) continue;

		Result.Add(Actor);
	}

	return Result;
}

void AAAAAPlayerController::ToggleLockOn()
{
	if (LockedTarget)
	{
		ClearLock();
		return;
	}

	TArray<AActor*> Candidates = GetLockOnCandidates();
	AActor* BestTarget = nullptr;
	float BestScore = MAX_flt;

	FVector CamLoc;
	FRotator CamRot;
	GetPlayerViewPoint(CamLoc, CamRot);
	FVector CamForward = CamRot.Vector();

	for (AActor* Candidate : Candidates)
	{
		FVector TargetChest = Candidate->GetActorLocation();
		float Distance = FVector::Distance(CamLoc, TargetChest);
		
		FVector DirToTarget = (TargetChest - CamLoc).GetSafeNormal();
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CamForward, DirToTarget)));
		
		float Score = AngleDegrees + Distance;
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
	{
		LockedTarget = BestTarget;
		TimeWithoutLOS = 0.0f;
		TimeLookInputFlick = 0.0f;
		OnLockTargetChanged.Broadcast(LockedTarget);
	}
}

void AAAAAPlayerController::CycleLockOnLeft()
{
	CycleLockOn(-1);
}

void AAAAAPlayerController::CycleLockOnRight()
{
	CycleLockOn(1);
}

void AAAAAPlayerController::CycleLockOn(int32 Dir)
{
	if (!LockedTarget) return;

	TArray<AActor*> Candidates = GetLockOnCandidates();
	Candidates.Remove(LockedTarget);

	AActor* BestTarget = nullptr;
	float BestScore = MAX_flt;

	FVector CamLoc;
	FRotator CamRot;
	GetPlayerViewPoint(CamLoc, CamRot);

	FVector CurrentDir = (LockedTarget->GetActorLocation() - CamLoc).GetSafeNormal();

	for (AActor* Candidate : Candidates)
	{
		FVector CandDir = (Candidate->GetActorLocation() - CamLoc).GetSafeNormal();
		
		float YawDiff = FMath::FindDeltaAngleDegrees(CurrentDir.Rotation().Yaw, CandDir.Rotation().Yaw);
		
		if ((Dir > 0 && YawDiff > 2.0f) || (Dir < 0 && YawDiff < -2.0f))
		{
			float Score = FMath::Abs(YawDiff);
			if (Score < BestScore)
			{
				BestScore = Score;
				BestTarget = Candidate;
			}
		}
	}

	if (BestTarget)
	{
		LockedTarget = BestTarget;
		TimeWithoutLOS = 0.0f;
		TimeLookInputFlick = 0.0f;
		OnLockTargetChanged.Broadcast(LockedTarget);
	}
}

void AAAAAPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (LockedTarget)
	{
		APawn* ControlledPawn = GetPawn();
		if (!ControlledPawn || !GetWorld())
		{
			ClearLock();
			return;
		}

		UAbilitySystemComponent* ASC = LockedTarget->FindComponentByClass<UAbilitySystemComponent>();
		if (!ASC)
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(LockedTarget))
			{
				ASC = ASI->GetAbilitySystemComponent();
			}
		}

		if (!ASC || ASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
		{
			ClearLock();
			return;
		}

		const UAAACombatSettings* Settings = UAAACombatSettings::Get();
		if (Settings)
		{
			float MaxRange = Settings->LockOnRange * 1.3f;
			if (FVector::Distance(ControlledPawn->GetActorLocation(), LockedTarget->GetActorLocation()) > MaxRange)
			{
				ClearLock();
				return;
			}
		}

		FVector PawnEye;
		FRotator PawnEyeRot;
		ControlledPawn->GetActorEyesViewPoint(PawnEye, PawnEyeRot);
		FVector TargetChest = LockedTarget->GetActorLocation();

		FHitResult HitResult;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(ControlledPawn);
		TraceParams.AddIgnoredActor(LockedTarget);
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, PawnEye, TargetChest, ECC_Visibility, TraceParams);

		if (bHit)
		{
			TimeWithoutLOS += DeltaTime;
			if (TimeWithoutLOS > 1.5f)
			{
				ClearLock();
				return;
			}
		}
		else
		{
			TimeWithoutLOS = 0.0f;
		}

		FVector CamLoc;
		FRotator CamRot;
		GetPlayerViewPoint(CamLoc, CamRot);
		FRotator TargetRot = (TargetChest - CamLoc).Rotation();
		
		TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -35.0f, 25.0f);

		float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CamRot.Vector(), TargetRot.Vector())));
		if (AngleDiff > 30.0f)
		{
			TimeLookInputFlick += DeltaTime;
			if (TimeLookInputFlick > 0.4f)
			{
				ClearLock();
				return;
			}
		}
		else
		{
			TimeLookInputFlick = 0.0f;
		}

		FRotator NewRot = FMath::RInterpTo(GetControlRotation(), TargetRot, DeltaTime, 8.0f);
		SetControlRotation(NewRot);
	}
}

#if !UE_BUILD_SHIPPING
#include "Effects/AAAGE_Damage.h"
#include "AAACombatSettings.h"

static FAutoConsoleCommandWithWorldAndArgs GHurtCmd(
	TEXT("rr.combat.hurt"),
	TEXT("rr.combat.hurt <damage> [poise]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) {
		if (World)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
					{
						UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
						if (ASC)
						{
							float Damage = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 10.0f;
							float Poise = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.0f;

							FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
							UGameplayEffect* GEDefault = UAAAGE_Damage::StaticClass()->GetDefaultObject<UGameplayEffect>();
							FGameplayEffectSpec* Spec = new FGameplayEffectSpec(GEDefault, Context, 1.0f);
							Spec->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_Damage, Damage);
							if (Poise > 0.0f)
							{
								Spec->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_PoiseDamage, Poise);
							}
							ASC->ApplyGameplayEffectSpecToSelf(*Spec);
						}
					}
				}
			}
		}
	})
);

static FAutoConsoleCommandWithWorldAndArgs GStaggerCmd(
	TEXT("rr.combat.stagger"),
	TEXT("rr.combat.stagger"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) {
		if (World)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (UAAACombatComponent* CombatComp = Pawn->FindComponentByClass<UAAACombatComponent>())
					{
						const UAAACombatSettings* Settings = UAAACombatSettings::Get();
						CombatComp->ApplyTimedState(FAAAGameplayTags::Get().State_Combat_Staggered, Settings->StaggerSeconds);
					}
				}
			}
		}
	})
);

static FAutoConsoleCommandWithWorldAndArgs GSetStaminaCmd(
	TEXT("rr.combat.setstamina"),
	TEXT("rr.combat.setstamina <v>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) {
		if (World && Args.Num() > 0)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
					{
						UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
						if (ASC)
						{
							for (const UAttributeSet* Set : ASC->GetSpawnedAttributes())
							{
								if (Set)
								{
									if (FProperty* Prop = FindFProperty<FProperty>(Set->GetClass(), FName("Stamina")))
									{
										ASC->SetNumericAttributeBase(FGameplayAttribute(Prop), FCString::Atof(*Args[0]));
									}
								}
							}
						}
					}
				}
			}
		}
	})
);
#endif
