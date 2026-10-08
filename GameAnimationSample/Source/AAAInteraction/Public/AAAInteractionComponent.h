// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AAAInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAAAOnInteractSignature, class AActor*, TargetActor);

/**
 * Foundation component for allowing a character to trace for and trigger 
 * the AAAInteractable interface on world objects.
 */
UCLASS(ClassGroup=(AAA), meta=(BlueprintSpawnableComponent))
class AAAINTERACTION_API UAAAInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAAAInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "AAA|Interaction")
	FAAAOnInteractSignature OnInteractExecuted;
	
	// Framework stub
	UFUNCTION(BlueprintCallable, Category = "AAA|Interaction")
	virtual void TryInteract();
};
