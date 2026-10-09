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
		// Characters whose Anim Blueprint has no DefaultSlot (Paragon: "FullBody" only) replay the montage's animation
		// on the slot they do have; otherwise the montage would run invisibly (notifies fire, nothing moves).
		if (!MontageSlotOverride.IsNone())
		{
			const FSlotAnimationTrack* Track = Montage->SlotAnimTracks.Num() > 0 ? &Montage->SlotAnimTracks[0] : nullptr;
			if (Track && Track->SlotName != MontageSlotOverride && Track->AnimTrack.AnimSegments.Num() > 0)
			{
				if (UAnimSequenceBase* Seq = Track->AnimTrack.AnimSegments[0].GetAnimReference())
				{
					const float Rate = PlayRate * Montage->RateScale;
					if (UAnimMontage* Dyn = AnimInst->PlaySlotAnimationAsDynamicMontage(Seq, MontageSlotOverride,
						Montage->BlendIn.GetBlendTime(), Montage->BlendOut.GetBlendTime(), Rate))
					{
						return Dyn->GetPlayLength() / FMath::Max(0.01f, Rate);
					}
				}
			}
		}
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

	// Heavy (perilous) hit: thrown down, then a real get-up instead of popping upright when the montage blends out.
	if (bHeavy && HitReactions.Knockdown)
	{
		const float Rate = 0.9f;
		PlayMontageOnOwner(HitReactions.Knockdown, Rate);
		if (HitReactions.GetUp && GetWorld())
		{
			const float EffRate = FMath::Max(0.05f, Rate * HitReactions.Knockdown->RateScale);
			const float Delay = FMath::Max(0.1f, HitReactions.Knockdown->GetPlayLength() / EffRate - 0.12f);
			GetWorld()->GetTimerManager().SetTimer(GetUpTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (!HasState(FAAAGameplayTags::Get().State_Combat_Dead))
				{
					PlayMontageOnOwner(HitReactions.GetUp, 1.0f);
				}
			}), Delay, false);
		}
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
		UAnimMontage* Montage = CurrentWeaponSettings->Animations.BlockMontage;
		FName StartSec = Montage->IsValidSectionName(FName("Loop")) ? FName("Loop") : NAME_None;
		PlayMontageOnOwner(Montage, 1.0f, StartSec);
		if (UAnimInstance* AnimInst = FindAnimInstance())
		{
			FName ActiveSec = StartSec.IsNone() ? (Montage->GetNumSections() > 0 ? Montage->GetSectionName(0) : FName("Default")) : StartSec;
			AnimInst->Montage_SetNextSection(ActiveSec, ActiveSec, Montage);
		}
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
	/** The mesh weapons attach to: retargeted display actor (VisualOverride), or ACharacter::GetMesh(). */
	USkeletalMeshComponent* FindOwnerMesh(AActor* Owner)
	{
		if (!Owner) return nullptr;

		// Characters with retargeted display actors (such as the robot on SandboxCharacter_CMC) must attach weapons
		// to the VISIBLE display mesh, otherwise the weapon sits on the hidden master mannequin.
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

		if (const ACharacter* Character = Cast<ACharacter>(Owner))
		{
			return Character->GetMesh();
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
	if (HandSocket.IsNone()) HandSocket = FName("weapon_r");

	if (OwnerMesh->DoesSocketExist(HandSocket))
	{
		CurrentWeapon->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
		CurrentWeapon->SetActorRelativeTransform(CurrentWeaponSettings->Visuals.HandOffset);
	}
	else
	{
		// Fallback: attach to hand_r and apply the standard weapon_r offset relative to wrist bone
		CurrentWeapon->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_r"));
		FTransform FallbackOffset(FRotator(0.0f, -180.0f, 0.0f), FVector(-10.764f, 5.811f, -0.728f), FVector::OneVector);
		CurrentWeapon->SetActorRelativeTransform(FallbackOffset * CurrentWeaponSettings->Visuals.HandOffset);
	}
	CurrentWeapon->SetActorHiddenInGame(false);
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
	CurrentWeapon->SetActorHiddenInGame(true);
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
		// Players holster only on purpose (1 / mouse wheel / R): the sword vanishes, so auto-sheath would look like a bug.
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn && OwnerPawn->IsPlayerControlled())
		{
			return;
		}
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
		BufferExpiresAt = GetWorld()->GetTimeSeconds() + (UAAACombatSettings::Get() ? UAAACombatSettings::Get()->ComboInputBufferSeconds : 0.8f);
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				FGameplayEventData EventData;
				EventData.EventTag = FAAAGameplayTags::Get().GameplayEvent_Combat_ComboInput;
				EventData.Instigator = GetOwner();
				ASC->HandleGameplayEvent(FAAAGameplayTags::Get().GameplayEvent_Combat_ComboInput, &EventData);
			}
		}
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
	// Called on the ATTACKER's component. Fan the hit out to the global listeners (knockback, hit-stop/shake/numbers,
	// match stats, boss deathblow) and make the VICTIM react.
	OnHitConfirmed.Broadcast(Attacker, Victim, Hit, bHeavy, Damage);
	FAAACombatEvents::OnHitConfirmed().Broadcast(Attacker, Victim, Hit, bHeavy, Damage);
	if (Victim)
	{
		if (UAAACombatComponent* VictimCombat = Victim->FindComponentByClass<UAAACombatComponent>())
		{
			if (VictimCombat->HasState(FAAAGameplayTags::Get().State_Combat_Blocking))
			{
				VictimCombat->PlayBlockImpact();
			}
			else
			{
				VictimCombat->PlayHitReaction(Attacker, bHeavy);
			}
		}
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

		// Local client responsiveness for blocking
		if (bWantsBlock)
		{
			if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
			{
				UAnimMontage* Montage = CurrentWeaponSettings->Animations.BlockMontage;
				if (Montage->CompositeSections.Num() > 0)
				{
					Montage->CompositeSections[0].NextSectionName = Montage->CompositeSections[0].SectionName;
				}
				FName StartSec = Montage->IsValidSectionName(FName("Loop")) ? FName("Loop") : NAME_None;
				PlayMontageOnOwner(Montage, 1.0f, StartSec);
				if (UAnimInstance* AnimInst = FindAnimInstance())
				{
					FName ActiveSec = StartSec.IsNone() ? (Montage->GetNumSections() > 0 ? Montage->GetSectionName(0) : FName("Default")) : StartSec;
					AnimInst->Montage_SetNextSection(ActiveSec, ActiveSec, Montage);
				}
			}
		}
		else
		{
			if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
			{
				if (UAnimInstance* AnimInst = FindAnimInstance())
				{
					if (AnimInst->Montage_IsPlaying(CurrentWeaponSettings->Animations.BlockMontage))
					{
						AnimInst->Montage_Stop(0.15f, CurrentWeaponSettings->Animations.BlockMontage);
					}
				}
			}
		}
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
			ASC->HasMatchingGameplayTag(Tags.State_Combat_Staggered))
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

		// Cancelling an active attack into a reactive block/parry
		bool bWasAttacking = ASC->HasMatchingGameplayTag(Tags.State_Combat_Attacking);
		if (bWasAttacking)
		{
			FGameplayTagContainer AttackTags;
			AttackTags.AddTag(Tags.InputTag_Attack);
			AttackTags.AddTag(Tags.InputTag_AttackHeavy);
			AttackTags.AddTag(Tags.State_Combat_Attacking);
			ASC->CancelAbilities(&AttackTags);
			ASC->RemoveLooseGameplayTag(Tags.State_Combat_Attacking);

			if (UAnimInstance* AnimInst = FindAnimInstance())
			{
				AnimInst->StopAllMontages(0.08f);
			}
			if (AAAAWeaponActor* Weapon = GetCurrentWeapon())
			{
				Weapon->StopWeaponTrace();
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
		if (bWasAttacking || (CurrentTime - LastParryWindowStart >= 0.15f))
		{
			LastParryWindowStart = CurrentTime;
			const UAAACombatSettings* Settings = UAAACombatSettings::Get();
			float ParryDuration = Settings ? Settings->ParryWindowSeconds : 0.35f;
			if (ParryDuration < 0.35f) ParryDuration = 0.35f;
			ApplyTimedState(Tags.State_Combat_Parrying, ParryDuration);
		}

		if (CurrentWeaponSettings && CurrentWeaponSettings->Animations.BlockMontage)
		{
			UAnimMontage* Montage = CurrentWeaponSettings->Animations.BlockMontage;
			if (Montage->CompositeSections.Num() > 0)
			{
				Montage->CompositeSections[0].NextSectionName = Montage->CompositeSections[0].SectionName;
			}
			FName StartSec = Montage->IsValidSectionName(FName("Loop")) ? FName("Loop") : NAME_None;
			PlayMontageOnOwner(Montage, 1.0f, StartSec);
			if (UAnimInstance* AnimInst = FindAnimInstance())
			{
				FName ActiveSec = StartSec.IsNone() ? (Montage->GetNumSections() > 0 ? Montage->GetSectionName(0) : FName("Default")) : StartSec;
				AnimInst->Montage_SetNextSection(ActiveSec, ActiveSec, Montage);
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
					AnimInst->Montage_Stop(0.15f, CurrentWeaponSettings->Animations.BlockMontage);
				}
			}
		}
	}
}

void UAAACombatComponent::OnParriedEvent(const FGameplayEventData* Payload)
{
	// This runs on the ATTACKER whose swing was deflected; Payload->Target is the defender.
	// Sekiro rule: a deflect costs the attacker posture (poise). Only when posture breaks is the attacker staggered (deathblow window);
	// otherwise its swing is knocked back (attack cancelled + short recoil).
	FAAACombatEvents::OnParried().Broadcast(GetOwner(), const_cast<AActor*>(Payload ? Payload->Target.Get() : nullptr));

	const FAAAGameplayTags& Tags = FAAAGameplayTags::Get();
	const UAAACombatSettings* Settings = UAAACombatSettings::Get();
	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}

	if (ASC)
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(Tags.State_Combat_Attacking);
		ASC->CancelAbilities(&CancelTags);
	}
	// montage-driven attacks (the boss) are not abilities: stop whatever swing is playing
	if (UAnimInstance* Anim = FindAnimInstance())
	{
		Anim->Montage_Stop(0.12f);
	}

	bool bBroken = true;
	if (ASC)
	{
		const FGameplayAttribute PoiseAttr = GetAttributeByName(ASC, FName("Poise"));
		const FGameplayAttribute MaxPoiseAttr = GetAttributeByName(ASC, FName("MaxPoise"));
		if (PoiseAttr.IsValid() && MaxPoiseAttr.IsValid())
		{
			const bool bPerilous = ASC->HasMatchingGameplayTag(Tags.State_Boss_Perilous);
			const float DeflectPoise = bPerilous ? 6.0f : 38.0f;
			const float NewPoise = ASC->GetNumericAttribute(PoiseAttr) - DeflectPoise;
			bBroken = NewPoise <= 0.0f;
			ASC->SetNumericAttributeBase(PoiseAttr, bBroken ? ASC->GetNumericAttribute(MaxPoiseAttr) : NewPoise);
			LastPoiseDrainTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		}
	}

	if (bBroken)
	{
		ApplyTimedState(Tags.State_Combat_Staggered, Settings->ParryStaggerSeconds * 2.0f);
		if (ASC)
		{
			FGameplayEventData Event;
			Event.EventTag = Tags.GameplayEvent_Combat_Staggered;
			ASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Staggered, &Event); // plays the stagger reaction + boss deathblow window
		}
	}
	else
	{
		ApplyTimedState(Tags.State_Combat_Staggered, 0.35f); // brief recoil: cannot attack straight back
		PlayMontageOnOwner(HitReactions.Front ? HitReactions.Front.Get() : HitReactions.Stagger.Get(), 1.4f);
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


