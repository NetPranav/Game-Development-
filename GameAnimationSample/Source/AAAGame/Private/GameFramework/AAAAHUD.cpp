// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "GameFramework/AAAAHUD.h"
#include "Blueprint/UserWidget.h"
#include "AAAHitFeedbackSubsystem.h"

AAAAAHUD::AAAAAHUD()
{
	// Default constructor
}

void AAAAAHUD::BeginPlay()
{
	Super::BeginPlay();

	// Initialize and spawn primary widgets here
}

bool AAAAAHUD::ToggleGameMenu()
{
	if (!GameMenuClass)
	{
		return false; // Cannot open menu if class is not assigned in Blueprint
	}

	// Create the widget if it doesn't exist yet
	if (!GameMenuWidget)
	{
		GameMenuWidget = CreateWidget<UUserWidget>(GetWorld(), GameMenuClass);
	}

	if (GameMenuWidget)
	{
		// If it's already on screen, remove it (closes menu)
		if (GameMenuWidget->IsInViewport())
		{
			GameMenuWidget->RemoveFromParent();
			return false; // Menu is now closed
		}
		else
		{
			// If it's not on screen, add it (opens menu). This fires 'Event Construct' in Blueprint!
			GameMenuWidget->AddToViewport(100); // High Z-order
			return true; // Menu is now open
		}
	}

	return false;
}

void AAAAAHUD::DrawHUD()
{
	Super::DrawHUD();

	if (UWorld* World = GetWorld())
	{
		if (UAAAHitFeedbackSubsystem* FeedbackSubsystem = World->GetSubsystem<UAAAHitFeedbackSubsystem>())
		{
			FeedbackSubsystem->DrawDamageNumbers(this);
		}
	}
}
