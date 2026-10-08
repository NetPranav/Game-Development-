#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AAASaveable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAAASaveable : public UInterface
{
	GENERATED_BODY()
};

class AAACORE_API IAAASaveable
{
	GENERATED_BODY()

public:
	// Add interface functions here
};
