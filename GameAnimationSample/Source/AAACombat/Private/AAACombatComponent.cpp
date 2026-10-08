// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAACombatComponent.h"
#include "Components/ChildActorComponent.h"
#include "AAAAWeaponActor.h"
#include "DataAssets/AAAWeaponSettings.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AAAGameplayTags.h"
#include "AAADebugLogger.h"
#include "AAACombatSettings.h"

#include "AAACombatEvents.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

static FGameplayAttribute GetAttributeByName(UAbilitySystemComponent* ASC, FName AttributeName)
{
	if (!ASC) return FGameplayAttribute();
	for (const UAttributeSet* Set : ASC->GetSpawnedAttributes())
	{
		if (Set)
		{
			if (FProperty* Prop = FindFProperty<FProperty>(Set->GetClass(), AttributeName))
			{
				return FGameplayAttribute(Prop);
			}
		}
	}
	return FGameplayAttribute();
}

UAAACombatComponent::UAAACombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	CombatStance = EAAACombatStance::Unarmed;
}

void UAAACombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAAACombatComponent, CurrentWeapon);
	DOREPLIFETIME(UAAACombatComponent, CurrentWeaponSettings);
	DOREPLIFETIME(UAAACombatComponent, CombatStance);
	DOREPLIFETIME(UAAACombatComponent, DamageMultiplier);
	DOREPLIFETIME(UAAACombatComponent, DamageReduction);
}

UAnimInstance* UAAACombatComponent::FindAnimInstance() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			return Mesh->GetAnimInstance();
		}
	}
	else if (AActor* Owner = GetOwner())
	{
		TInlineComponentArray<USkeletalMeshComponent*> Meshes;
		Owner->GetComponents(Meshes);
		for (USkeletalMeshComponent* Mesh : Meshes)
		{
			// Must be directly owned by this actor (not a child actor's component)
			if (Mesh && Mesh->GetOwner() == Owner)
			{
				return Mesh->GetAnimInstance();
			}
		}
	}
	return nullptr;
}

float UAAACombatComponent::PlayMontageOnOwner(UAnimMontage* Montage, float PlayRate, FName StartSection)
{
	if (!Montage) return 0.0f;
	if (UAnimInstance* AnimInst = FindAnimInstance())
	{
		float Duration = AnimInst->Montage_Play(Montage, PlayRate);
		if (Duration > 0.f && StartSection != NAME_None)
		{
			AnimInst->Montage_JumpToSection(StartSection, Montage);
		}
		return Duration;
	}
	return 0.0f;
}

void UAAACombatComponent::PlayHitReaction(AActor* Attacker, bool bHeavy)
{
	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
	if (HasState(Tags.State_Combat_Dead) || HasState(Tags.State_Combat_Staggered))
	{
		return;
	}
	if (HasState(Tags.State_Combat_Attacking) && !bHeavy)
	{
		return;
	}

	UAnimMontage* MontageToPlay = HitReactions.Front;
	if (Attacker && GetOwner())
	{
		FVector ToAttacker = (Attacker->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();
		FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
		FVector Right = GetOwner()->GetActorRightVector().GetSafeNormal2D();

		float ForwardDot = FVector::DotProduct(Forward, ToAttacker);
		float RightDot = FVector::DotProduct(Right, ToAttacker);

		if (ForwardDot >= 0.5f)
		{
			MontageToPlay = HitReactions.Front;
		}
		else if (ForwardDot <= -0.5f)
		{
			MontageToPlay = HitReactions.Back;
		}
		else if (RightDot > 0.0f)
		{
			MontageToPlay = HitReactions.Right;
		}
		else
		{
			MontageToPlay = HitReactions.Left;
		}
	}

	if (MontageToPlay)
	{
		float Rate = bHeavy ? 0.9f : 1.2f;
		PlayMontageOnOwner(MontageToPlay, Rate);
	}
}

void UAAACombatComponent::PlayBlockImpact()
{
	if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
	{
		PlayMontageOnOwner(CurrentWeaponSettings->Animations.BlockMontage, 1.0f, FName("Loop"));
	}
	else if (HitReactions.Front)
	{
		PlayMontageOnOwner(HitReactions.Front, 1.0f);
	}
}

void UAAACombatComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeRegen();
}

void UAAACombatComponent::EquipWeapon(UAAAWeaponSettings* NewWeaponSettings)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// Destroy old weapon if exists
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	CurrentWeaponSettings = NewWeaponSettings;

	if (!NewWeaponSettings)
	{
		CombatStance = EAAACombatStance::Unarmed;
		UpdateAbilitySystemTags();
		return;
	}

	// Spawn the new weapon actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<AAAAWeaponActor>(AAAAWeaponActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->InitializeWeapon(CurrentWeaponSettings);

		// Default to Drawn in hand on first equip
		CombatStance = EAAACombatStance::Armed_Drawn;
		UpdateWeaponAttachment();
		UpdateAbilitySystemTags();
		RefreshCombatTimeout(8.0f);

		FAAADebugLogger::LogAction(TEXT("EQUIP"), FString::Printf(TEXT("Equipped: %s (Base Damage: %.1f)"), *CurrentWeaponSettings->GetName(), CurrentWeaponSettings->Combat.BaseDamage), FColor::Yellow);

		// Apply Granted Abilities and Effects to the owner's ASC
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				for (auto AbilityClass : CurrentWeaponSettings->Abilities.GrantedAbilities)
				{
					if (AbilityClass)
					{
						ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, GetOwner()));
					}
				}

				for (auto EffectClass : CurrentWeaponSettings->Abilities.GrantedEffects)
				{
					if (EffectClass)
					{
						FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
						Context.AddInstigator(GetOwner(), GetOwner());
						ASC->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
					}
				}
			}
		}
	}
}

void UAAACombatComponent::DrawWeapon()
{
	if (!CurrentWeaponSettings)
	{
		// Fallback: try loading test sword if available
		UAAAWeaponSettings* TestSword = Cast<UAAAWeaponSettings>(StaticLoadObject(UAAAWeaponSettings::StaticClass(), nullptr, TEXT("/Game/AAA/DataAssets/Asset/Sword/DA_TestSword")));
		if (TestSword)
		{
			CurrentWeaponSettings = TestSword;
		}
	}

	if (!CurrentWeapon && CurrentWeaponSettings)
	{
		EquipWeapon(CurrentWeaponSettings);
		return;
	}

	if (!CurrentWeaponSettings || !CurrentWeapon)
	{
		CombatStance = EAAACombatStance::Unarmed;
		UpdateAbilitySystemTags();
		FAAADebugLogger::LogAction(TEXT("STANCE"), TEXT("Unarmed / Barehanded Mode"), FColor::Silver);
		return;
	}

	CombatStance = EAAACombatStance::Armed_Drawn;
	UpdateAbilitySystemTags();
	RefreshCombatTimeout(8.0f);

	FAAADebugLogger::LogAction(TEXT("WEAPON"), TEXT("Stance: DRAWN (Weapon in Hand)"), FColor::Cyan);

	if (CurrentWeaponSettings->Animations.EquipAnimation)
	{
		PlayMontageOnOwner(CurrentWeaponSettings->Animations.EquipAnimation);
	}
	else
	{
		AttachWeaponToHand();
	}
}

void UAAACombatComponent::SheathWeapon()
{
	if (!CurrentWeaponSettings || !CurrentWeapon)
	{
		CombatStance = EAAACombatStance::Unarmed;
		UpdateAbilitySystemTags();
		return;
	}

	CombatStance = EAAACombatStance::Armed_Holstered;
	UpdateAbilitySystemTags();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSheathTimerHandle);
	}

	FAAADebugLogger::LogAction(TEXT("WEAPON"), TEXT("Stance: SHEATHED (Weapon on Back)"), FColor(100, 200, 255));

	if (CurrentWeaponSettings->Animations.UnequipAnimation)
	{
		PlayMontageOnOwner(CurrentWeaponSettings->Animations.UnequipAnimation);
	}
	else
	{
		AttachWeaponToHolster();
	}
}

void UAAACombatComponent::ToggleWeaponDraw()
{
	if (CombatStance == EAAACombatStance::Armed_Drawn)
	{
		SheathWeapon();
	}
	else
	{
		DrawWeapon();
	}
}

namespace
{
	/** The mesh weapons attach to: ACharacter::GetMesh(), or (Mover pawns, which are plain APawns) the first skeletal mesh owned by the actor. */
	USkeletalMeshComponent* FindOwnerMesh(AActor* Owner)
	{
		if (const ACharacter* Character = Cast<ACharacter>(Owner))
		{
			return Character->GetMesh();
		}
		if (!Owner) return nullptr;

		// Mover pawns show a retargeted display actor (VisualOverride child actor) while the hidden master mannequin only drives the pose.
		// The weapon must follow the VISIBLE hand, otherwise it drifts away from it whenever the retargeted pose differs from the mannequin's.
		TInlineComponentArray<UChildActorComponent*> Children;
		Owner->GetComponents(Children);
		for (UChildActorComponent* Child : Children)
		{
			if (Child && Child->GetName().Contains(TEXT("VisualOverride")))
			{
				if (AActor* Display = Child->GetChildActor())
				{
					USkeletalMeshComponent* DisplayMesh = Display->FindComponentByClass<USkeletalMeshComponent>();
					if (DisplayMesh && DisplayMesh->GetSkeletalMeshAsset())
					{
						return DisplayMesh;
					}
				}
			}
		}
		return Owner->FindComponentByClass<USkeletalMeshComponent>();
	}
}

void UAAACombatComponent::AttachWeaponToHand()
{
	if (!CurrentWeapon || !CurrentWeaponSettings) return;

	USkeletalMeshComponent* OwnerMesh = FindOwnerMesh(GetOwner());
	if (!OwnerMesh) return;

	FName HandSocket = CurrentWeaponSettings->Visuals.AttachmentSocket;
	if (HandSocket.IsNone()) HandSocket = FName("hand_r");

	CurrentWeapon->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
	CurrentWeapon->SetActorRelativeTransform(CurrentWeaponSettings->Visuals.HandOffset);
}

void UAAACombatComponent::AttachWeaponToHolster()
{
	if (!CurrentWeapon || !CurrentWeaponSettings) return;

	USkeletalMeshComponent* OwnerMesh = FindOwnerMesh(GetOwner());
	if (!OwnerMesh) return;

	FName HolsterSocket = CurrentWeaponSettings->Visuals.HolsterSocket;
	if (HolsterSocket.IsNone()) HolsterSocket = FName("spine_03");

	CurrentWeapon->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HolsterSocket);
	CurrentWeapon->SetActorRelativeTransform(CurrentWeaponSettings->Visuals.HolsterOffset);
}

void UAAACombatComponent::RefreshCombatTimeout(float TimeoutSeconds)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSheathTimerHandle);
		if (TimeoutSeconds > 0.0f && CombatStance == EAAACombatStance::Armed_Drawn)
		{
			World->GetTimerManager().SetTimer(AutoSheathTimerHandle, this, &UAAACombatComponent::SheathWeapon, TimeoutSeconds, false);
			FAAADebugLogger::LogStatus(101, TEXT("COMBAT"), TEXT("In Combat Stance (Auto-sheaths in 8s of inactivity)"), FColor::Orange);
		}
	}
}

void UAAACombatComponent::UpdateWeaponAttachment()
{
	if (CombatStance == EAAACombatStance::Armed_Drawn)
	{
		AttachWeaponToHand();
	}
	else if (CombatStance == EAAACombatStance::Armed_Holstered)
	{
		AttachWeaponToHolster();
	}
}

void UAAACombatComponent::RequestAttack(EAAAAttackType Type)
{
	if (!HasState(FAAAGameplayTags::Get().State_Combat_Attacking))
	{
		RequestedAttackType = Type;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(Type == EAAAAttackType::Light ? FAAAGameplayTags::Get().InputTag_Attack : FAAAGameplayTags::Get().InputTag_AttackHeavy);
				ASC->TryActivateAbilitiesByTag(TagContainer);
			}
		}
	}
	else
	{
		BufferedAttack = Type;
		BufferExpiresAt = GetWorld()->GetTimeSeconds() + UAAACombatSettings::Get()->ComboInputBufferSeconds;
	}
}

bool UAAACombatComponent::ConsumeBufferedAttack(EAAAAttackType& OutType)
{
	if (GetWorld()->GetTimeSeconds() <= BufferExpiresAt)
	{
		OutType = BufferedAttack;
		BufferExpiresAt = 0.0f;
		return true;
	}
	return false;
}

bool UAAACombatComponent::IsAttacking() const
{
	return HasState(FAAAGameplayTags::Get().State_Combat_Attacking);
}

void UAAACombatComponent::SetLoadoutModifiers(float DamageMult, float DamageReduct)
{
	if (GetOwner()->HasAuthority())
	{
		DamageMultiplier = DamageMult;
		DamageReduction = DamageReduct;
	}
}

void UAAACombatComponent::Multicast_OnHitConfirmed_Implementation(AActor* Attacker, AActor* Victim, const FHitResult& Hit, bool bHeavy, float Damage)
{
	OnHitConfirmed.Broadcast(Attacker, Victim, Hit, bHeavy, Damage);
	if (Victim == GetOwner())
	{
		PlayHitReaction(Attacker, bHeavy);
	}
}

void UAAACombatComponent::UpdateAbilitySystemTags()
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();

			ASC->RemoveLooseGameplayTag(Tags.State_Combat_Drawn);
			ASC->RemoveLooseGameplayTag(Tags.State_Combat_Holstered);
			ASC->RemoveLooseGameplayTag(Tags.State_Combat_Unarmed);

			switch (CombatStance)
			{
			case EAAACombatStance::Armed_Drawn:
				ASC->AddLooseGameplayTag(Tags.State_Combat_Drawn);
				break;
			case EAAACombatStance::Armed_Holstered:
				ASC->AddLooseGameplayTag(Tags.State_Combat_Holstered);
				break;
			case EAAACombatStance::Unarmed:
			default:
				ASC->AddLooseGameplayTag(Tags.State_Combat_Unarmed);
				break;
			}
		}
	}
}

void UAAACombatComponent::OnRep_CurrentWeapon()
{
	UpdateWeaponAttachment();
}

void UAAACombatComponent::OnRep_CombatStance()
{
	UpdateWeaponAttachment();
	UpdateAbilitySystemTags();
}

void UAAACombatComponent::ApplyTimedState(FGameplayTag Tag, float Seconds)
{
	if (!GetOwner()->HasAuthority()) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(Tag, 1, EGameplayTagReplicationState::TagOnly);
			
			if (Seconds > 0.0f)
			{
				FTimerHandle& Handle = StateTimers.FindOrAdd(Tag);
				TWeakObjectPtr<UAAACombatComponent> WeakThis(this);
				GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakThis, Tag]()
				{
					if (WeakThis.IsValid())
					{
						WeakThis->ClearState(Tag);
					}
				}), Seconds, false);
			}
		}
	}
}

void UAAACombatComponent::ClearState(FGameplayTag Tag)
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(Tag, 1, EGameplayTagReplicationState::TagOnly);
		}
	}
	
	if (FTimerHandle* Handle = StateTimers.Find(Tag))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Handle);
		StateTimers.Remove(Tag);
	}
}

bool UAAACombatComponent::HasState(FGameplayTag Tag) const
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			return ASC->HasMatchingGameplayTag(Tag);
		}
	}
	return false;
}

void UAAACombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSheathTimerHandle);
		World->GetTimerManager().ClearTimer(RegenTimerHandle);
		World->GetTimerManager().ClearTimer(InitRetryTimerHandle);
		
		for (auto& Pair : StateTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
				if (FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(Tags.GameplayEvent_Combat_Parried))
				{
					Delegate->Remove(ParriedEventHandle);
				}
				if (FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(Tags.GameplayEvent_Combat_Blocked))
				{
					Delegate->Remove(BlockedEventHandle);
				}
				if (FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(Tags.GameplayEvent_Combat_Staggered))
				{
					Delegate->Remove(StaggeredEventHandle);
				}
				if (FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(Tags.GameplayEvent_Combat_Death))
				{
					Delegate->Remove(DeathEventHandle);
				}
				ASC->RegisterGameplayTagEvent(Tags.State_Combat_Blocking, EGameplayTagEventType::NewOrRemoved).Remove(BlockingTagEventHandle);
			}
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void UAAACombatComponent::InitializeRegen()
{
	if (!GetOwner()->HasAuthority()) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayAttribute StaminaAttr = GetAttributeByName(ASC, FName("Stamina"));
			FGameplayAttribute PoiseAttr = GetAttributeByName(ASC, FName("Poise"));

			if (StaminaAttr.IsValid() && PoiseAttr.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttr).AddUObject(this, &UAAACombatComponent::OnStaminaChanged);
				ASC->GetGameplayAttributeValueChangeDelegate(PoiseAttr).AddUObject(this, &UAAACombatComponent::OnPoiseChanged);

				const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
				ParriedEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(Tags.GameplayEvent_Combat_Parried).AddUObject(this, &UAAACombatComponent::OnParriedEvent);
				BlockedEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(Tags.GameplayEvent_Combat_Blocked).AddUObject(this, &UAAACombatComponent::OnBlockedEvent);
				StaggeredEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(Tags.GameplayEvent_Combat_Staggered).AddUObject(this, &UAAACombatComponent::OnStaggeredEvent);
				DeathEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(Tags.GameplayEvent_Combat_Death).AddUObject(this, &UAAACombatComponent::OnDeathEvent);
				BlockingTagEventHandle = ASC->RegisterGameplayTagEvent(Tags.State_Combat_Blocking, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAAACombatComponent::OnBlockingTagChanged);

				GetWorld()->GetTimerManager().SetTimer(RegenTimerHandle, this, &UAAACombatComponent::RegenTick, 0.1f, true);
				return;
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(InitRetryTimerHandle, this, &UAAACombatComponent::InitializeRegen, 0.5f, false);
}

void UAAACombatComponent::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < Data.OldValue)
	{
		LastStaminaDrainTime = GetWorld()->GetTimeSeconds();
	}
}

void UAAACombatComponent::OnPoiseChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < Data.OldValue)
	{
		LastPoiseDrainTime = GetWorld()->GetTimeSeconds();
	}
}

void UAAACombatComponent::RegenTick()
{
	if (!GetOwner()->HasAuthority()) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
			const UAAACombatSettings* Settings = UAAACombatSettings::Get();
			float CurrentTime = GetWorld()->GetTimeSeconds();

			FGameplayAttribute StaminaAttr = GetAttributeByName(ASC, FName("Stamina"));
			FGameplayAttribute MaxStaminaAttr = GetAttributeByName(ASC, FName("MaxStamina"));
			FGameplayAttribute PoiseAttr = GetAttributeByName(ASC, FName("Poise"));
			FGameplayAttribute MaxPoiseAttr = GetAttributeByName(ASC, FName("MaxPoise"));

			if (StaminaAttr.IsValid() && MaxStaminaAttr.IsValid())
			{
				bool bCanRegenStamina = !ASC->HasMatchingGameplayTag(Tags.State_Combat_Blocking) && !ASC->HasMatchingGameplayTag(Tags.State_Combat_Attacking);
				if (bCanRegenStamina && (CurrentTime - LastStaminaDrainTime) >= Settings->StaminaRegenDelay)
				{
					bool bFound = false;
					float CurrentStamina = ASC->GetGameplayAttributeValue(StaminaAttr, bFound);
					float MaxStamina = ASC->GetGameplayAttributeValue(MaxStaminaAttr, bFound);
					if (CurrentStamina < MaxStamina)
					{
						ASC->SetNumericAttributeBase(StaminaAttr, FMath::Min(CurrentStamina + (Settings->StaminaRegenPerSecond * 0.1f), MaxStamina));
					}
				}
			}

			if (PoiseAttr.IsValid() && MaxPoiseAttr.IsValid())
			{
				if ((CurrentTime - LastPoiseDrainTime) >= Settings->PoiseRegenDelay)
				{
					bool bFound = false;
					float CurrentPoise = ASC->GetGameplayAttributeValue(PoiseAttr, bFound);
					float MaxPoise = ASC->GetGameplayAttributeValue(MaxPoiseAttr, bFound);
					if (CurrentPoise < MaxPoise)
					{
						ASC->SetNumericAttributeBase(PoiseAttr, FMath::Min(CurrentPoise + (Settings->PoiseRegenPerSecond * 0.1f), MaxPoise));
					}
				}
			}
		}
	}
}


void UAAACombatComponent::SetBlocking(bool bWantsBlock)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_SetBlocking(bWantsBlock);
	}
	else
	{
		Server_SetBlocking_Implementation(bWantsBlock);
	}
}

bool UAAACombatComponent::Server_SetBlocking_Validate(bool bWantsBlock)
{
	return true;
}

void UAAACombatComponent::Server_SetBlocking_Implementation(bool bWantsBlock)
{
	if (!GetOwner()->HasAuthority()) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!ASI) return;
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();

	if (bWantsBlock)
	{
		if (ASC->HasMatchingGameplayTag(Tags.State_Combat_Dead) ||
			ASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered) ||
			ASC->HasMatchingGameplayTag(Tags.State_Combat_Attacking))
		{
			return;
		}

		FGameplayAttribute StaminaAttr = GetAttributeByName(ASC, FName("Stamina"));
		if (StaminaAttr.IsValid())
		{
			bool bFound = false;
			float Stamina = ASC->GetGameplayAttributeValue(StaminaAttr, bFound);
			if (bFound && Stamina <= 0.0f)
			{
				return; // Cannot block with 0 stamina
			}
		}

		if (CombatStance == EAAACombatStance::Armed_Holstered || CombatStance == EAAACombatStance::Unarmed)
		{
			if (CurrentWeaponSettings)
			{
				DrawWeapon();
			}
		}

		ASC->AddLooseGameplayTag(Tags.State_Combat_Blocking, 1, EGameplayTagReplicationState::TagOnly);
		bIsBlocking = true;

		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastParryWindowStart >= 0.4f)
		{
			LastParryWindowStart = CurrentTime;
			const UAAACombatSettings* Settings = UAAACombatSettings::Get();
			ApplyTimedState(Tags.State_Combat_Parrying, Settings->ParryWindowSeconds);
		}

		if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
		{
			if (UAnimInstance* AnimInst = FindAnimInstance())
			{
				AnimInst->Montage_Play(CurrentWeaponSettings->Animations.BlockMontage);
				if (CurrentWeaponSettings->Animations.BlockMontage->IsValidSectionName(FName("Loop")))
				{
					AnimInst->Montage_JumpToSection(FName("Loop"), CurrentWeaponSettings->Animations.BlockMontage);
				}
			}
		}
	}
	else
	{
		ASC->RemoveLooseGameplayTag(Tags.State_Combat_Blocking, 1, EGameplayTagReplicationState::TagOnly);
		ASC->RemoveLooseGameplayTag(Tags.State_Combat_Parrying, 1, EGameplayTagReplicationState::TagOnly);
		bIsBlocking = false;

		if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
		{
			if (UAnimInstance* AnimInst = FindAnimInstance())
			{
				if (AnimInst->Montage_IsPlaying(CurrentWeaponSettings->Animations.BlockMontage))
				{
					AnimInst->Montage_Stop(0.2f, CurrentWeaponSettings->Animations.BlockMontage);
				}
			}
		}
	}
}

void UAAACombatComponent::OnParriedEvent(const FGameplayEventData* Payload)
{
	// This runs on the ATTACKER whose swing was deflected; Payload->Target is the defender.
	FAAACombatEvents::OnParried().Broadcast(GetOwner(), const_cast<AActor*>(Payload ? Payload->Target.Get() : nullptr));
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayTagContainer CancelTags;
			CancelTags.AddTag(FAAAGameplayTags::Get().State_Combat_Attacking);
			ASC->CancelAbilities(&CancelTags);
		}
	}

	const UAAACombatSettings* Settings = UAAACombatSettings::Get();
	ApplyTimedState(FAAAGameplayTags::Get().State_Combat_Staggered, Settings->ParryStaggerSeconds);
	
	if (HitReactions.Stagger)
	{
		PlayMontageOnOwner(HitReactions.Stagger);
	}
}

void UAAACombatComponent::OnBlockedEvent(const FGameplayEventData* Payload)
{
	FAAACombatEvents::OnBlocked().Broadcast(nullptr, GetOwner());
	PlayBlockImpact();
}

void UAAACombatComponent::OnStaggeredEvent(const FGameplayEventData* Payload)
{
	FAAACombatEvents::OnStaggered().Broadcast(GetOwner());
	if (HitReactions.Stagger)
	{
		PlayMontageOnOwner(HitReactions.Stagger);
	}
}

void UAAACombatComponent::OnDeathEvent(const FGameplayEventData* Payload)
{
	FAAACombatEvents::OnDeath().Broadcast(GetOwner());
	if (HitReactions.Death)
	{
		PlayMontageOnOwner(HitReactions.Death);
	}
}

void UAAACombatComponent::OnBlockingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0 && bIsBlocking)
	{
		bIsBlocking = false;
		
		if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
		{
			if (UAnimInstance* AnimInst = FindAnimInstance())
			{
				if (AnimInst->Montage_IsPlaying(CurrentWeaponSettings->Animations.BlockMontage))
				{
					AnimInst->Montage_Stop(0.2f, CurrentWeaponSettings->Animations.BlockMontage);
				}
			}
		}
	}
}


