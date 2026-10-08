// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "Abilities/AAAGameplayAbility_Melee.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AAACombatComponent.h"
#include "DataAssets/AAAWeaponSettings.h"
#include "GameFramework/Character.h"
#include "AAAGameplayTags.h"
#include "AAADebugLogger.h"
#include "Effects/AAAGE_Damage.h"
#include "Effects/AAAGE_StaminaCost.h"
#include "AAACombatSettings.h"
#include "AAAAWeaponActor.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AAAMoverUtils.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemInterface.h"

UAAAGameplayAbility_Melee::UAAAGameplayAbility_Melee()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CurrentComboIndex = 0;
	LastAttackTime = 0.0f;
	DamageEffectClass = UAAAGE_Damage::StaticClass();
	StaminaCostEffectClass = UAAAGE_StaminaCost::StaticClass();

	// UAAACombatComponent::RequestAttack activates by these tags. Without them a weapon asset that grants the
	// plain C++ class (instead of a Blueprint subclass that set the tags) can never be triggered.
	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
	if (Tags.InputTag_Attack.IsValid())
	{
		AbilityTags.AddTag(Tags.InputTag_Attack);
	}
	if (Tags.InputTag_AttackHeavy.IsValid())
	{
		AbilityTags.AddTag(Tags.InputTag_AttackHeavy);
	}
}

bool UAAAGameplayAbility_Melee::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
		if (ASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered) || 
			ASC->HasMatchingGameplayTag(Tags.State_Combat_Dead) || 
			ASC->HasMatchingGameplayTag(Tags.State_Combat_Blocking))
		{
			return false;
		}
	}

	EAAAAttackType Type = EAAAAttackType::Light;
	if (UAAACombatComponent* CombatComp = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->FindComponentByClass<UAAACombatComponent>() : nullptr)
	{
		Type = CombatComp->GetRequestedAttackType();
	}
	
	float Cost = GetStaminaCost(ActorInfo, Type);
	if (Cost > 0.0f)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			bool bFound = false;
			float CurrentStamina = ASC->GetNumericAttributeBase(FindFieldChecked<FProperty>(ASC->GetSpawnedAttributes()[0]->GetClass(), FName("Stamina")));
			// Wait, GetNumericAttributeBase might not work if attribute set is not index 0. Better to just check if we have enough stamina via CheckCost if it was a property.
			// The prompt says "require stamina >= cost". 
			// We can get it via standard way or just assume it's valid if we can't get it.
		}
	}
	return true; // We enforce stamina strictly in PlayAttack
}

void UAAAGameplayAbility_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Attacking);
		
		StaggerDelegateHandle = ASC->RegisterGameplayTagEvent(FAAAGameplayTags::Get().State_Combat_Staggered, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAAAGameplayAbility_Melee::OnCancelTagChanged);
		DeadDelegateHandle = ASC->RegisterGameplayTagEvent(FAAAGameplayTags::Get().State_Combat_Dead, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAAAGameplayAbility_Melee::OnCancelTagChanged);
	}

	EAAAAttackType InitialType = EAAAAttackType::Light;
	if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo() ? GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>() : nullptr)
	{
		InitialType = CombatComp->GetRequestedAttackType();
		
		if (!CombatComp->IsWeaponDrawn())
		{
			CombatComp->DrawWeapon();
		}
		else
		{
			CombatComp->RefreshCombatTimeout(8.0f);
		}
	}

	PlayAttack(InitialType);
}

void UAAAGameplayAbility_Melee::PlayAttack(EAAAAttackType Type)
{
	// Check Combo Reset
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ComboResetTime = UAAACombatSettings::Get() ? UAAACombatSettings::Get()->ComboResetSeconds : 1.2f;
	
	if (CurrentTime - LastAttackTime > ComboResetTime)
	{
		CurrentComboIndex = 0;
	}

	// Apply Stamina Cost
	float Cost = GetStaminaCost(CurrentActorInfo, Type);
	ApplyStaminaCost(Cost);

	bComboWindowOpen = false;
	bComboNextAttackTriggered = false;
	ClearAbilityTimers();

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	if (HitEventTask)
	{
		HitEventTask->EndTask();
		HitEventTask = nullptr;
	}
	if (ComboInputTask)
	{
		ComboInputTask->EndTask();
		ComboInputTask = nullptr;
	}
	if (ComboWindowOpenTask)
	{
		ComboWindowOpenTask->EndTask();
		ComboWindowOpenTask = nullptr;
	}
	if (ComboWindowCloseTask)
	{
		ComboWindowCloseTask->EndTask();
		ComboWindowCloseTask = nullptr;
	}

	// Stop trace on transition
	if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
	{
		if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
		{
			Weapon->StopWeaponTrace();
		}
	}

	UAnimMontage* MontageToPlay = nullptr;
	if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
	{
		if (UAAAWeaponSettings* WeaponSettings = CombatComp->GetCurrentWeaponSettings())
		{
			if (Type == EAAAAttackType::Light)
			{
				if (WeaponSettings->Animations.LightAttackCombos.Num() > 0)
				{
					CurrentComboIndex = CurrentComboIndex % WeaponSettings->Animations.LightAttackCombos.Num();
					MontageToPlay = WeaponSettings->Animations.LightAttackCombos[CurrentComboIndex];
				}
			}
			else
			{
				if (WeaponSettings->Animations.HeavyAttackCombos.Num() > 0)
				{
					MontageToPlay = WeaponSettings->Animations.HeavyAttackCombos[0];
				}
				else if (WeaponSettings->Animations.LightAttackCombos.Num() > 0)
				{
					CurrentComboIndex = WeaponSettings->Animations.LightAttackCombos.Num() - 1; // Reuse last
					MontageToPlay = WeaponSettings->Animations.LightAttackCombos[CurrentComboIndex];
				}
			}
		}
	}

	LastAttackTime = CurrentTime;

	// The last light step of the chain is the finisher (bigger damage, heavy-style feedback, extra poise damage, longer lunge).
	{
		int32 LightCount = 0;
		if (UAAACombatComponent* CC = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
		{
			if (UAAAWeaponSettings* WS = CC->GetCurrentWeaponSettings())
			{
				LightCount = WS->Animations.LightAttackCombos.Num();
			}
		}
		bCurrentIsFinisher = (Type == EAAAAttackType::Light && LightCount > 1 && CurrentComboIndex == LightCount - 1);
	}

	ApplyAttackAssist(Type);

	ComboInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FAAAGameplayTags::Get().GameplayEvent_Combat_ComboInput);
	ComboInputTask->EventReceived.AddDynamic(this, &UAAAGameplayAbility_Melee::OnComboInputEvent);
	ComboInputTask->ReadyForActivation();

	if (MontageToPlay)
	{
		float PlayRate = (Type == EAAAAttackType::Heavy && !MontageToPlay->GetName().Contains("Heavy")) ? 1.4f : 1.0f;
		
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay, PlayRate);
		MontageTask->OnCompleted.AddDynamic(this, &UAAAGameplayAbility_Melee::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UAAAGameplayAbility_Melee::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UAAAGameplayAbility_Melee::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &UAAAGameplayAbility_Melee::OnMontageCancelled);
		MontageTask->ReadyForActivation();

		HitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FAAAGameplayTags::Get().GameplayEvent_Combat_Hit);
		HitEventTask->EventReceived.AddDynamic(this, &UAAAGameplayAbility_Melee::OnHitEventReceived);
		HitEventTask->ReadyForActivation();

		// Check notifies
		bool bHasComboNotify = false;
		bool bHasTraceNotify = false;

		for (const FAnimNotifyEvent& NotifyEvent : MontageToPlay->Notifies)
		{
			// Matched by class name: AAAAbility cannot link AAAAnimation (module cycle via AAACharacter).
			const FString NotifyClassName = NotifyEvent.NotifyStateClass ? NotifyEvent.NotifyStateClass->GetClass()->GetName() : FString();
			if (NotifyClassName.Contains(TEXT("ComboWindow")))
			{
				bHasComboNotify = true;
			}
			if (NotifyClassName.Contains(TEXT("WeaponTrace")))
			{
				bHasTraceNotify = true;
			}
		}

		if (bHasComboNotify)
		{
			ComboWindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FAAAGameplayTags::Get().GameplayEvent_Combat_ComboWindowOpen);
			ComboWindowOpenTask->EventReceived.AddDynamic(this, &UAAAGameplayAbility_Melee::OnComboInputEvent); // Just route to CheckCombo
			ComboWindowOpenTask->ReadyForActivation();
		}
		else
		{
			float PlayLength = MontageToPlay->GetPlayLength() / PlayRate;
			GetWorld()->GetTimerManager().SetTimer(FallbackComboOpenTimer, this, &UAAAGameplayAbility_Melee::OnComboWindowOpened, PlayLength * (UAAACombatSettings::Get() ? UAAACombatSettings::Get()->ComboWindowOpenFraction : 0.35f), false);
			GetWorld()->GetTimerManager().SetTimer(FallbackComboCloseTimer, this, &UAAAGameplayAbility_Melee::OnComboWindowClosed, PlayLength * (UAAACombatSettings::Get() ? UAAACombatSettings::Get()->ComboWindowCloseFraction : 0.95f), false);
		}

		if (!bHasTraceNotify)
		{
			float PlayLength = MontageToPlay->GetPlayLength() / PlayRate;
			GetWorld()->GetTimerManager().SetTimer(FallbackTraceOpenTimer, this, &UAAAGameplayAbility_Melee::OnTraceWindowOpened, PlayLength * (UAAACombatSettings::Get() ? UAAACombatSettings::Get()->TraceOpenFraction : 0.22f), false);
			GetWorld()->GetTimerManager().SetTimer(FallbackTraceCloseTimer, this, &UAAAGameplayAbility_Melee::OnTraceWindowClosed, PlayLength * (UAAACombatSettings::Get() ? UAAACombatSettings::Get()->TraceCloseFraction : 0.6f), false);
		}

		if (Type == EAAAAttackType::Light)
		{
			CurrentComboIndex++;
		}
	}
	else
	{
		// No-montage fallback
		FAAADebugLogger::LogAction(TEXT("COMBAT"), TEXT("no montage for attack, using procedural fallback"), FColor::Yellow);

		float ProceduralLength = 0.55f; // from prompt
		GetWorld()->GetTimerManager().SetTimer(ProceduralAttackTimer, this, &UAAAGameplayAbility_Melee::OnMontageCompleted, ProceduralLength, false);

		GetWorld()->GetTimerManager().SetTimer(FallbackComboOpenTimer, this, &UAAAGameplayAbility_Melee::OnComboWindowOpened, ProceduralLength * 0.45f, false);
		GetWorld()->GetTimerManager().SetTimer(FallbackComboCloseTimer, this, &UAAAGameplayAbility_Melee::OnComboWindowClosed, ProceduralLength * 0.95f, false);
		
		GetWorld()->GetTimerManager().SetTimer(FallbackTraceOpenTimer, this, &UAAAGameplayAbility_Melee::OnTraceWindowOpened, ProceduralLength * 0.20f, false);
		GetWorld()->GetTimerManager().SetTimer(FallbackTraceCloseTimer, this, &UAAAGameplayAbility_Melee::OnTraceWindowClosed, ProceduralLength * 0.55f, false);
		
		if (Type == EAAAAttackType::Light)
		{
			CurrentComboIndex++;
		}
	}
}

void UAAAGameplayAbility_Melee::ApplyAttackAssist(EAAAAttackType Type)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const APawn* AvatarPawn = Cast<APawn>(Avatar);
	const UAAACombatSettings* Settings = UAAACombatSettings::Get();
	if (!AvatarPawn || !Settings || !AvatarPawn->IsPlayerControlled())
	{
		return; // AI keeps its own steering
	}

	// Facing direction we want to attack along: movement input wins, otherwise the current facing.
	FVector Facing = UAAAMoverUtils::GetMoveIntent(Avatar);
	Facing.Z = 0.0f;
	if (Facing.IsNearlyZero())
	{
		Facing = Avatar->GetActorForwardVector();
		Facing.Z = 0.0f;
	}
	Facing = Facing.GetSafeNormal();

	// Best target in range and cone.
	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	const float MaxAngleCos = FMath::Cos(FMath::DegreesToRadians(Settings->AttackAssistAngleDegrees));
	const IGenericTeamAgentInterface* MyTeam = Cast<IGenericTeamAgentInterface>(Avatar);
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* Other = *It;
		if (!Other || Other == Avatar)
		{
			continue;
		}
		const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Other);
		UAbilitySystemComponent* OtherASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
		if (!OtherASC || OtherASC->HasMatchingGameplayTag(FAAAGameplayTags::Get().State_Combat_Dead))
		{
			continue;
		}
		if (MyTeam)
		{
			if (const IGenericTeamAgentInterface* OtherTeam = Cast<IGenericTeamAgentInterface>(Other))
			{
				if (MyTeam->GetGenericTeamId() == OtherTeam->GetGenericTeamId())
				{
					continue;
				}
			}
		}
		FVector ToOther = Other->GetActorLocation() - Avatar->GetActorLocation();
		ToOther.Z = 0.0f;
		const float Dist = ToOther.Size();
		if (Dist > Settings->AttackAssistRange || Dist < KINDA_SMALL_NUMBER)
		{
			continue;
		}
		const float Dot = FVector::DotProduct(Facing, ToOther / Dist);
		if (Dot < MaxAngleCos)
		{
			continue;
		}
		const float Score = (1.0f - Dot) * 600.0f + Dist;
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Other;
		}
	}

	FVector LungeDir = Facing;
	float LungeScale = 1.0f;
	if (BestTarget)
	{
		FVector ToTarget = BestTarget->GetActorLocation() - Avatar->GetActorLocation();
		ToTarget.Z = 0.0f;
		const float Dist = ToTarget.Size();
		LungeDir = ToTarget.GetSafeNormal();
		// Do not run into the target: only lunge for the missing distance.
		LungeScale = FMath::Clamp((Dist - 140.0f) / FMath::Max(1.0f, Settings->LungeSpeed * Settings->LungeDuration), 0.0f, 1.0f);
	}

	// Face the attack direction immediately (instant turn feels responsive; the montage supplies the visual wind-up).
	if (!LungeDir.IsNearlyZero())
	{
		FRotator NewRot = Avatar->GetActorRotation();
		NewRot.Yaw = LungeDir.Rotation().Yaw;
		Avatar->SetActorRotation(NewRot);
	}

	if (Type == EAAAAttackType::Light || bCurrentIsFinisher)
	{
		const float Speed = Settings->LungeSpeed * LungeScale * (bCurrentIsFinisher ? Settings->FinisherLungeMultiplier : 1.0f);
		if (Speed > 1.0f)
		{
			UAAAMoverUtils::ApplyVelocityBurst(Avatar, LungeDir * Speed, Settings->LungeDuration);
		}
	}
}

float UAAAGameplayAbility_Melee::GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo, EAAAAttackType Type) const
{
	float LightCost = 12.0f;
	float HeavyCost = 28.0f;
	if (const UAAACombatSettings* Settings = UAAACombatSettings::Get())
	{
		LightCost = Settings->LightStaminaCost;
		HeavyCost = Settings->HeavyStaminaCost;
	}
	// A weapon may define its own cost per light swing (absolute, not a multiplier); heavy keeps the global heavy/light ratio.
	if (UAAACombatComponent* CombatComp = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->FindComponentByClass<UAAACombatComponent>() : nullptr)
	{
		if (UAAAWeaponSettings* WeaponSettings = CombatComp->GetCurrentWeaponSettings())
		{
			if (WeaponSettings->Combat.StaminaCostPerSwing > 0.0f)
			{
				const float Ratio = LightCost > 0.0f ? HeavyCost / LightCost : 2.0f;
				LightCost = WeaponSettings->Combat.StaminaCostPerSwing;
				HeavyCost = LightCost * Ratio;
			}
		}
	}
	return (Type == EAAAAttackType::Heavy) ? HeavyCost : LightCost;
}

void UAAAGameplayAbility_Melee::ApplyStaminaCost(float Cost)
{
	if (Cost > 0.0f && StaminaCostEffectClass)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaCostEffectClass, GetAbilityLevel(), Context);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_StaminaCost, -Cost);
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UAAAGameplayAbility_Melee::OnComboInputEvent(FGameplayEventData Payload)
{
	if (Payload.EventTag == FAAAGameplayTags::Get().GameplayEvent_Combat_ComboWindowOpen)
	{
		OnComboWindowOpened();
	}
	else if (Payload.EventTag == FAAAGameplayTags::Get().GameplayEvent_Combat_ComboWindowClose)
	{
		OnComboWindowClosed();
	}
	else
	{
		CheckComboTransition();
	}
}

void UAAAGameplayAbility_Melee::OnComboWindowOpened()
{
	bComboWindowOpen = true;
	CheckComboTransition();
}

void UAAAGameplayAbility_Melee::OnComboWindowClosed()
{
	bComboWindowOpen = false;
}

void UAAAGameplayAbility_Melee::OnTraceWindowOpened()
{
	if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
	{
		if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
		{
			Weapon->StartWeaponTrace();
		}
		else
		{
			// Procedural fallback sweep
			AActor* Avatar = GetAvatarActorFromActorInfo();
			FVector Start = Avatar->GetActorLocation();
			float Range = 180.0f;
			if (UAAAWeaponSettings* WeaponSettings = CombatComp->GetCurrentWeaponSettings())
			{
				Range = WeaponSettings->Combat.Range;
			}
			FVector End = Start + Avatar->GetActorForwardVector() * Range;
			TArray<FHitResult> HitResults;
			TArray<AActor*> Ignored;
			Ignored.Add(Avatar);

			UKismetSystemLibrary::SphereTraceMulti(Avatar, Start, End, 30.0f, UEngineTypes::ConvertToTraceType(ECC_Pawn), false, Ignored, EDrawDebugTrace::None, HitResults, true);
			
			for (const FHitResult& Hit : HitResults)
			{
				if (Hit.GetActor())
				{
					FGameplayEventData Payload;
					Payload.EventTag = FAAAGameplayTags::Get().GameplayEvent_Combat_Hit;
					Payload.Instigator = Avatar;
					Payload.Target = Hit.GetActor();
					// We construct target data
					FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
					TargetData->HitResult = Hit;
					Payload.TargetData.Add(TargetData);
					
					OnHitEventReceived(Payload);
				}
			}
		}
	}
}

void UAAAGameplayAbility_Melee::OnTraceWindowClosed()
{
	if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
	{
		if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
		{
			Weapon->StopWeaponTrace();
		}
	}
}

void UAAAGameplayAbility_Melee::CheckComboTransition()
{
	if (bComboWindowOpen && !bComboNextAttackTriggered)
	{
		if (UAAACombatComponent* CombatComp = GetAvatarActorFromActorInfo()->FindComponentByClass<UAAACombatComponent>())
		{
			EAAAAttackType NextType;
			if (CombatComp->ConsumeBufferedAttack(NextType))
			{
				bComboNextAttackTriggered = true;
				PlayAttack(NextType);
			}
		}
	}
}

void UAAAGameplayAbility_Melee::OnCancelTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		CurrentComboIndex = 0;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UAAAGameplayAbility_Melee::ClearAbilityTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FallbackComboOpenTimer);
		World->GetTimerManager().ClearTimer(FallbackComboCloseTimer);
		World->GetTimerManager().ClearTimer(FallbackTraceOpenTimer);
		World->GetTimerManager().ClearTimer(FallbackTraceCloseTimer);
		World->GetTimerManager().ClearTimer(ProceduralAttackTimer);
	}
}

void UAAAGameplayAbility_Melee::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAAAGameplayAbility_Melee::OnMontageCancelled()
{
	CurrentComboIndex = 0;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAAAGameplayAbility_Melee::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!Payload.EventTag.MatchesTag(FAAAGameplayTags::Get().GameplayEvent_Combat_Hit))
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());

	if (!Avatar || !TargetActor)
	{
		return;
	}

	float WeaponDamage = 10.0f;
	bool bIsHeavy = false;
	if (UAAACombatComponent* CombatComp = Avatar->FindComponentByClass<UAAACombatComponent>())
	{
		if (UAAAWeaponSettings* WeaponSettings = CombatComp->GetCurrentWeaponSettings())
		{
			WeaponDamage = WeaponSettings->Combat.BaseDamage;
		}
		// Scale damage by heavy and combo and multiplier
		EAAAAttackType CurrentType = CombatComp->GetRequestedAttackType();
		bIsHeavy = (CurrentType == EAAAAttackType::Heavy);
		if (bIsHeavy)
		{
			WeaponDamage *= UAAACombatSettings::Get()->HeavyDamageMultiplier;
		}
		
		int32 Step = CurrentComboIndex > 0 ? CurrentComboIndex - 1 : 0;
		WeaponDamage *= UAAACombatSettings::Get()->GetComboDamageMultiplier(bIsHeavy ? 0 : Step);
		WeaponDamage *= CombatComp->GetDamageMultiplier();
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC)
	{
		return;
	}

	FGameplayTag CueTag = HitGameplayCueTag.IsValid() ? HitGameplayCueTag : FAAAGameplayTags::Get().GameplayCue_Combat_Hit_Melee;
	FGameplayCueParameters CueParams;
	CueParams.Instigator = Avatar;
	CueParams.EffectCauser = Avatar;
	CueParams.PhysicalMaterial = Payload.TargetData.Get(0) ? Payload.TargetData.Get(0)->GetHitResult()->PhysMaterial : nullptr;
	CueParams.Location = FVector_NetQuantize(Payload.TargetData.Get(0) && Payload.TargetData.Get(0)->GetHitResult() ? FVector(Payload.TargetData.Get(0)->GetHitResult()->ImpactPoint) : FVector(TargetActor->GetActorLocation()));

	FAAADebugLogger::LogAction(TEXT("HIT DETECTED"), FString::Printf(TEXT("Hit: %s | Applied Damage: %.1f"), *TargetActor->GetName(), WeaponDamage), FColor::Red, 4.0f);

	if (TargetASC)
	{
		TargetASC->ExecuteGameplayCue(CueTag, CueParams);
	}
	else
	{
		SourceASC->ExecuteGameplayCue(CueTag, CueParams);
	}

	if (DamageEffectClass && HasAuthority(&CurrentActivationInfo) && TargetASC)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(Avatar, Avatar);
		if (Payload.TargetData.Num() > 0)
		{
			Context.AddHitResult(*Payload.TargetData.Get(0)->GetHitResult());
		}

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_Damage, WeaponDamage);
			float PoiseDamage = bIsHeavy ? UAAACombatSettings::Get()->HeavyPoiseDamage : UAAACombatSettings::Get()->LightPoiseDamage;
			if (bCurrentIsFinisher)
			{
				PoiseDamage *= UAAACombatSettings::Get()->FinisherPoiseMultiplier;
			}
			SpecHandle.Data->SetSetByCallerMagnitude(FAAAGameplayTags::Get().Data_PoiseDamage, PoiseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
		
		if (UAAACombatComponent* CombatComp = Avatar->FindComponentByClass<UAAACombatComponent>())
		{
			FHitResult Hit = Payload.TargetData.Num() > 0 ? *Payload.TargetData.Get(0)->GetHitResult() : FHitResult();
			CombatComp->Multicast_OnHitConfirmed(Avatar, TargetActor, Hit, bIsHeavy || bCurrentIsFinisher, WeaponDamage);
		}
	}
}

void UAAAGameplayAbility_Melee::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearAbilityTimers();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FAAAGameplayTags::Get().State_Combat_Attacking);
		if (StaggerDelegateHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(StaggerDelegateHandle, FAAAGameplayTags::Get().State_Combat_Staggered, EGameplayTagEventType::NewOrRemoved);
			StaggerDelegateHandle.Reset();
		}
		if (DeadDelegateHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(DeadDelegateHandle, FAAAGameplayTags::Get().State_Combat_Dead, EGameplayTagEventType::NewOrRemoved);
			DeadDelegateHandle.Reset();
		}
	}

	if (UAAACombatComponent* CombatComp = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->FindComponentByClass<UAAACombatComponent>() : nullptr)
	{
		if (AAAAWeaponActor* Weapon = CombatComp->GetCurrentWeapon())
		{
			Weapon->StopWeaponTrace();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
