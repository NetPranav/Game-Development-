// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Registry/UnrealAITool.h"

class FUnrealAITool_GetProjectInfo : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_project_info"); }
	virtual FString GetDescription() const override { return TEXT("Get summary information about the current Unreal project, engine version, and content directories."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_FindAsset : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("find_asset"); }
	virtual FString GetDescription() const override { return TEXT("Search for assets in the project by name query, asset class, or directory path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_GetAssetInfo : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_asset_info"); }
	virtual FString GetDescription() const override { return TEXT("Get detailed metadata, class info, dependencies, and referencers for a specific asset path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_CreateAssetFolder : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("create_asset_folder"); }
	virtual FString GetDescription() const override { return TEXT("Create a new directory folder under /Game content path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_MoveAsset : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("move_asset"); }
	virtual FString GetDescription() const override { return TEXT("Move an asset from source package path to destination package path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_RenameAsset : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("rename_asset"); }
	virtual FString GetDescription() const override { return TEXT("Rename an existing asset in place."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_DeleteAsset : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("delete_asset"); }
	virtual FString GetDescription() const override { return TEXT("Delete an asset at the specified package path."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::Destructive; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_SaveAsset : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("save_asset"); }
	virtual FString GetDescription() const override { return TEXT("Save a specific asset package or all dirty packages in the editor."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};
