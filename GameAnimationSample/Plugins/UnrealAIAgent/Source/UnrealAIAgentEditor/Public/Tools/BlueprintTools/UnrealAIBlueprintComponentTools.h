// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Registry/UnrealAITool.h"

class FUnrealAITool_AddComponent : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("add_component"); }
	virtual FString GetDescription() const override { return TEXT("Add a component to a Blueprint SimpleConstructionScript hierarchy (e.g. StaticMeshComponent, BoxComponent, SphereComponent, AudioComponent, CameraComponent, SpringArmComponent, PointLightComponent)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_SetComponentProperty : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("set_component_property"); }
	virtual FString GetDescription() const override { return TEXT("Configure component properties such as assigning a StaticMesh asset or setting relative transform."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};
