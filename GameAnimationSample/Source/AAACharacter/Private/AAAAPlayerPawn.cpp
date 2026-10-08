// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAAAPlayerPawn.h"

// Framework Components
#include "AAAAbilitySystemComponent.h"
#include "AAAAttributeSet.h"
#include "AAAAttributeComponent.h"
#include "AAATeamComponent.h"
#include "AAAReplicationComponent.h"
#include "AAAInventoryComponent.h"
#include "AAAEquipmentComponent.h"
#include "AAAInteractionComponent.h"
#include "AAACombatComponent.h"
#include "AAAAbilityInputBindingComponent.h"

// Data Assets
#include "DataAssets/AAACharacterSettings.h"
#include "DataAssets/AAAWeaponSettings.h"

// Abilities
#include "Abilities/AAAGameplayAbility_Melee.h"
#include "Abilities/AAAGameplayAbility_Dodge.h"

// Framework (PlayerState)
#include "GameFramework/AAAAPlayerState.h"

// Enhanced Input
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AAAAAPlayerPawn::AAAAAPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// *** ASC and AttributeSet are NOT created here ***
	// They are owned by AAAAAPlayerState (Lyra Pattern)
	// This Pawn fetches them during PossessedBy/OnRep_PlayerState

	// Core Character Modules
	AttributeComponent = CreateDefaultSubobject<UAAAAttributeComponent>(TEXT("AttributeComponent"));
	TeamComponent = CreateDefaultSubobject<UAAATeamComponent>(TEXT("TeamComponent"));
	ReplicationComponent = CreateDefaultSubobject<UAAAReplicationComponent>(TEXT("ReplicationComponent"));

	// Inventory & Equipment Modules
	InventoryComponent = CreateDefaultSubobject<UAAAInventoryComponent>(TEXT("InventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UAAAEquipmentComponent>(TEXT("EquipmentComponent"));

	// Interaction Module
	InteractionComponent = CreateDefaultSubobject<UAAAInteractionComponent>(TEXT("InteractionComponent"));

	// Combat Module
	CombatComponent = CreateDefaultSubobject<UAAACombatComponent>(TEXT("CombatComponent"));

	// Ability Input Binding Module (Lyra Pattern)
	AbilityInputBindingComponent = CreateDefaultSubobject<UAAAAbilityInputBindingComponent>(TEXT("AbilityInputBindingComponent"));
}

UAbilitySystemComponent* AAAAAPlayerPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void AAAAAPlayerPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AAAAAPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	// Auto-equip test weapon if available and no weapon is currently held
	if (HasAuthority() && CombatComponent && !CombatComponent->GetCurrentWeapon())
	{
		UAAAWeaponSettings* TestSword = Cast<UAAAWeaponSettings>(StaticLoadObject(UAAAWeaponSettings::StaticClass(), nullptr, TEXT("/Game/AAA/DataAssets/Asset/Sword/DA_TestSword")));
		if (TestSword)
		{
			CombatComponent->EquipWeapon(TestSword);
		}
	}
}

void AAAAAPlayerPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side: Fetch ASC from PlayerState and initialize
	InitAbilityActorInfoFromPlayerState();

	// Grant startup abilities from the Data Asset (server-only)
	if (HasAuthority() && CharacterSettings)
	{
		if (AAAAAPlayerState* PS = GetPlayerState<AAAAAPlayerState>())
		{
			PS->GrantStartupAbilities(
				CharacterSettings->Abilities.DefaultAbilities,
				CharacterSettings->Abilities.DefaultEffects,
				CharacterSettings->Abilities.StartingTags
			);
		}
	}
}

void AAAAAPlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side: Fetch ASC from PlayerState and initialize
	InitAbilityActorInfoFromPlayerState();
}

void AAAAAPlayerPawn::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// The CharacterSettings asset is assigned in BP_PlayerCharacter
			if (CharacterSettings)
			{
				if (CharacterSettings->Input.DefaultMappingContext)
				{
					Subsystem->AddMappingContext(CharacterSettings->Input.DefaultMappingContext, 0);
				}
				else
				{
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("CRITICAL ERROR: Your DA_PlayerCharacterSettings has no IMC_AAA_Main assigned in the Input category!"));
				}
			}
			else
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("CRITICAL ERROR: You forgot to assign DA_PlayerCharacterSettings to BP_PlayerCharacter!"));
			}
		}
	}

	// Wire up tag-based ability activation via Enhanced Input
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (AbilityInputBindingComponent && CharacterSettings)
		{
			AbilityInputBindingComponent->SetupBindings(EIC, CharacterSettings->Input.AbilityInputBindings);
		}
	}
}

void AAAAAPlayerPawn::InitAbilityActorInfoFromPlayerState()
{
	AAAAAPlayerState* PS = GetPlayerState<AAAAAPlayerState>();
	if (!PS) return;

	UAAAAbilitySystemComponent* ASC = PS->GetAAAAbilitySystemComponent();
	if (!ASC) return;

	// Cache the pointers for fast access
	AbilitySystemComponent = ASC;
	AttributeSet = PS->GetAttributeSet();

	// Initialize ASC Actor Info: Owner = PlayerState, Avatar = this Pawn
	// This is the Lyra pattern — the PlayerState owns the data, the Pawn is the physical avatar
	ASC->InitAbilityActorInfo(PS, this);

	// Grant core abilities (Melee & Dodge) if on authority
	if (HasAuthority())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(UAAAGameplayAbility_Melee::StaticClass(), 1, INDEX_NONE, this));
		ASC->GiveAbility(FGameplayAbilitySpec(UAAAGameplayAbility_Dodge::StaticClass(), 1, INDEX_NONE, this));
	}

	// Hook up the Attribute wrapper component to the ASC
	if (AttributeComponent)
	{
		AttributeComponent->InitializeWithAbilitySystem(ASC);
		AttributeComponent->OnDeath.AddUniqueDynamic(this, &AAAAAPlayerPawn::OnDeath);
	}

	// Pass ASC to AbilityInputBindingComponent so it can activate abilities by tag
	if (AbilityInputBindingComponent)
	{
		AbilityInputBindingComponent->SetAbilitySystemComponent(ASC);
	}

	// Notify the ASC that initialization is complete
	ASC->OnAbilitySystemInitialized();
}

void AAAAAPlayerPawn::OnDeath()
{
	// Disable Input
	DisableInput(Cast<APlayerController>(GetController()));

	// Disable movement logic (prevent Epic's Mover from updating)
	SetActorTickEnabled(false);
}
