// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Registry/UnrealAITool.h"

class FUnrealAITool_ListBlueprintGraphs : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("list_blueprint_graphs"); }
	virtual FString GetDescription() const override { return TEXT("List all graphs contained in a Blueprint (EventGraph, Function graphs, Macro graphs)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_CreateVariable : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("create_variable"); }
	virtual FString GetDescription() const override { return TEXT("Create a new member variable on a Blueprint with type and optional default value."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_ModifyVariable : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("modify_variable"); }
	virtual FString GetDescription() const override { return TEXT("Modify an existing Blueprint member variable's name, type, default value, or category."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_DeleteVariable : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("delete_variable"); }
	virtual FString GetDescription() const override { return TEXT("Delete a member variable from a Blueprint."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_CreateFunction : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("create_function"); }
	virtual FString GetDescription() const override { return TEXT("Create a new function graph on a Blueprint."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_DeleteFunction : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("delete_function"); }
	virtual FString GetDescription() const override { return TEXT("Delete a function graph from a Blueprint."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};
