// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

RRGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogRR, Log, All);

class FRRGameModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
