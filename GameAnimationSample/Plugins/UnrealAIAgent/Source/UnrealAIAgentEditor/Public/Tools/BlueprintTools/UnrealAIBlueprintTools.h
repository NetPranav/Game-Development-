// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Registry/UnrealAITool.h"

class FUnrealAITool_CreateBlueprint : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("create_blueprint"); }
	virtual FString GetDescription() const override { return TEXT("Create a new Blueprint asset with specified parent class and folder path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_OpenBlueprint : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("open_blueprint"); }
	virtual FString GetDescription() const override { return TEXT("Open a Blueprint asset in the Unreal Blueprint Editor."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_InspectBlueprint : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("inspect_blueprint"); }
	virtual FString GetDescription() const override { return TEXT("Inspect the full structural definition of a Blueprint (ParentClass, Components, Variables, Functions, Graphs)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_SaveBlueprint : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("save_blueprint"); }
	virtual FString GetDescription() const override { return TEXT("Save a Blueprint asset and mark its package clean."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};
