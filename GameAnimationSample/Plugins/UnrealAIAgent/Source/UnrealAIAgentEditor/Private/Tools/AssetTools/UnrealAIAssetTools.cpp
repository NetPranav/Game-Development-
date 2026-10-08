// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Tools/AssetTools/UnrealAIAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/App.h"
#include "Engine/Blueprint.h"

// -----------------------------------------------------------------------------
// get_project_info
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetProjectInfo::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	Schema->SetObjectField(TEXT("properties"), MakeShared<FJsonObject>());
	return Schema;
}

bool FUnrealAITool_GetProjectInfo::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("project_name"), FApp::GetProjectName());
	ResultObj->SetStringField(TEXT("engine_version"), FEngineVersion::Current().ToString());
	ResultObj->SetStringField(TEXT("root_content_path"), TEXT("/Game"));

	TArray<TSharedPtr<FJsonValue>> RootDirs;
	RootDirs.Add(MakeShared<FJsonValueString>(TEXT("/Game")));
	RootDirs.Add(MakeShared<FJsonValueString>(TEXT("/Engine")));
	ResultObj->SetArrayField(TEXT("content_roots"), RootDirs);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// find_asset
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_FindAsset::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> SearchQueryProp = MakeShared<FJsonObject>();
	SearchQueryProp->SetStringField(TEXT("type"), TEXT("string"));
	SearchQueryProp->SetStringField(TEXT("description"), TEXT("Optional name substring or pattern to match."));
	Props->SetObjectField(TEXT("search_query"), SearchQueryProp);

	TSharedPtr<FJsonObject> AssetClassProp = MakeShared<FJsonObject>();
	AssetClassProp->SetStringField(TEXT("type"), TEXT("string"));
	AssetClassProp->SetStringField(TEXT("description"), TEXT("Optional class type, e.g. Blueprint, StaticMesh, Material, Texture2D, SoundCue."));
	Props->SetObjectField(TEXT("asset_class"), AssetClassProp);

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Package path to search in, e.g. /Game or /Game/Weapons. Default is /Game."));
	Props->SetObjectField(TEXT("package_path"), PathProp);

	TSharedPtr<FJsonObject> LimitProp = MakeShared<FJsonObject>();
	LimitProp->SetStringField(TEXT("type"), TEXT("integer"));
	LimitProp->SetStringField(TEXT("description"), TEXT("Maximum number of results to return (default 50)."));
	Props->SetObjectField(TEXT("limit"), LimitProp);

	Schema->SetObjectField(TEXT("properties"), Props);
	return Schema;
}

bool FUnrealAITool_FindAsset::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString PackagePath = TEXT("/Game");
	if (InArgs->HasField(TEXT("package_path")))
	{
		PackagePath = InArgs->GetStringField(TEXT("package_path"));
	}

	FString SearchQuery;
	if (InArgs->HasField(TEXT("search_query")))
	{
		SearchQuery = InArgs->GetStringField(TEXT("search_query"));
	}

	FString AssetClass;
	if (InArgs->HasField(TEXT("asset_class")))
	{
		AssetClass = InArgs->GetStringField(TEXT("asset_class"));
	}

	int32 Limit = 50;
	if (InArgs->HasField(TEXT("limit")))
	{
		Limit = InArgs->GetIntegerField(TEXT("limit"));
	}

	FARFilter Filter;
	Filter.PackagePaths.Add(FName(*PackagePath));
	Filter.bRecursivePaths = true;

	if (!AssetClass.IsEmpty())
	{
		Filter.ClassPaths.Add(FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(*AssetClass)));
	}

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	TArray<TSharedPtr<FJsonValue>> MatchedAssets;
	int32 Count = 0;

	for (const FAssetData& Asset : AssetList)
	{
		const FString AssetName = Asset.AssetName.ToString();
		const FString AssetPath = Asset.GetObjectPathString();

		if (!SearchQuery.IsEmpty() && !AssetName.Contains(SearchQuery, ESearchCase::IgnoreCase))
		{
			continue;
		}

		TSharedPtr<FJsonObject> AssetObj = MakeShared<FJsonObject>();
		AssetObj->SetStringField(TEXT("name"), AssetName);
		AssetObj->SetStringField(TEXT("path"), AssetPath);
		AssetObj->SetStringField(TEXT("class"), Asset.AssetClassPath.GetAssetName().ToString());
		AssetObj->SetStringField(TEXT("package"), Asset.PackageName.ToString());

		MatchedAssets.Add(MakeShared<FJsonValueObject>(AssetObj));
		Count++;
		if (Count >= Limit)
		{
			break;
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetNumberField(TEXT("total_found"), MatchedAssets.Num());
	ResultObj->SetArrayField(TEXT("assets"), MatchedAssets);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// get_asset_info
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetAssetInfo::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> AssetPathProp = MakeShared<FJsonObject>();
	AssetPathProp->SetStringField(TEXT("type"), TEXT("string"));
	AssetPathProp->SetStringField(TEXT("description"), TEXT("Full asset path (e.g. /Game/Weapons/BP_Sword or /Game/Weapons/BP_Sword.BP_Sword)."));
	Props->SetObjectField(TEXT("asset_path"), AssetPathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("asset_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_GetAssetInfo::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString AssetPath = InArgs->GetStringField(TEXT("asset_path"));

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(AssetPath));
	if (!AssetData.IsValid())
	{
		// Try package name
		TArray<FAssetData> AssetsInPackage;
		AssetRegistry.GetAssetsByPackageName(FName(*AssetPath), AssetsInPackage);
		if (AssetsInPackage.Num() > 0)
		{
			AssetData = AssetsInPackage[0];
		}
	}

	if (!AssetData.IsValid())
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("ASSET_NOT_FOUND"), FString::Printf(TEXT("Asset '%s' not found."), *AssetPath));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
	ResultObj->SetStringField(TEXT("class"), AssetData.AssetClassPath.GetAssetName().ToString());
	ResultObj->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
	ResultObj->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());

	// Dependencies
	TArray<FName> Dependencies;
	AssetRegistry.GetDependencies(AssetData.PackageName, Dependencies);
	TArray<TSharedPtr<FJsonValue>> DepArray;
	for (const FName& Dep : Dependencies)
	{
		DepArray.Add(MakeShared<FJsonValueString>(Dep.ToString()));
	}
	ResultObj->SetArrayField(TEXT("dependencies"), DepArray);

	// Referencers
	TArray<FName> Referencers;
	AssetRegistry.GetReferencers(AssetData.PackageName, Referencers);
	TArray<TSharedPtr<FJsonValue>> RefArray;
	for (const FName& Ref : Referencers)
	{
		RefArray.Add(MakeShared<FJsonValueString>(Ref.ToString()));
	}
	ResultObj->SetArrayField(TEXT("referencers"), RefArray);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// create_asset_folder
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CreateAssetFolder::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> FolderPathProp = MakeShared<FJsonObject>();
	FolderPathProp->SetStringField(TEXT("type"), TEXT("string"));
	FolderPathProp->SetStringField(TEXT("description"), TEXT("Package folder path under /Game (e.g. /Game/Weapons/Swords)."));
	Props->SetObjectField(TEXT("folder_path"), FolderPathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("folder_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CreateAssetFolder::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString FolderPath = InArgs->GetStringField(TEXT("folder_path"));
	if (!FolderPath.StartsWith(TEXT("/Game")))
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("INVALID_PATH"), TEXT("Folder path must start with /Game."));
		return false;
	}

	FString FilenameOnDisk = FPackageName::LongPackageNameToFilename(FolderPath);
	if (!IFileManager::Get().MakeDirectory(*FilenameOnDisk, true))
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("FILESYSTEM_ERROR"), FString::Printf(TEXT("Failed to create folder on disk: %s"), *FilenameOnDisk));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("folder_path"), FolderPath);
	ResultObj->SetStringField(TEXT("disk_path"), FilenameOnDisk);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// move_asset
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_MoveAsset::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> SrcProp = MakeShared<FJsonObject>();
	SrcProp->SetStringField(TEXT("type"), TEXT("string"));
	SrcProp->SetStringField(TEXT("description"), TEXT("Source asset package path (e.g. /Game/OldFolder/BP_Sword)."));
	Props->SetObjectField(TEXT("source_path"), SrcProp);

	TSharedPtr<FJsonObject> DstProp = MakeShared<FJsonObject>();
	DstProp->SetStringField(TEXT("type"), TEXT("string"));
	DstProp->SetStringField(TEXT("description"), TEXT("Destination asset package path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("destination_path"), DstProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("source_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("destination_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_MoveAsset::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString SourcePath = InArgs->GetStringField(TEXT("source_path"));
	FString DestPath = InArgs->GetStringField(TEXT("destination_path"));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	TArray<FAssetRenameData> AssetsToRename;
	FAssetRenameData RenameData;
	RenameData.Asset = TSoftObjectPtr<UObject>(FSoftObjectPath(SourcePath)).LoadSynchronous();

	if (!RenameData.Asset.IsValid())
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("ASSET_NOT_FOUND"), FString::Printf(TEXT("Could not load source asset at '%s'."), *SourcePath));
		return false;
	}

	FString DestPackagePath = FPackageName::GetLongPackagePath(DestPath);
	FString DestAssetName = FPackageName::GetShortName(DestPath);

	RenameData.NewPackagePath = DestPackagePath;
	RenameData.NewName = DestAssetName;
	AssetsToRename.Add(RenameData);

	bool bSuccess = AssetToolsModule.Get().RenameAssets(AssetsToRename);
	if (!bSuccess)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("RENAME_FAILED"), FString::Printf(TEXT("Failed to move asset from '%s' to '%s'."), *SourcePath, *DestPath));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("source_path"), SourcePath);
	ResultObj->SetStringField(TEXT("new_path"), DestPath);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// rename_asset
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_RenameAsset::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("asset_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("New short asset name (e.g. BP_Excalibur)."));
	Props->SetObjectField(TEXT("new_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("asset_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("new_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_RenameAsset::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString AssetPath = InArgs->GetStringField(TEXT("asset_path"));
	FString NewName = InArgs->GetStringField(TEXT("new_name"));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	UObject* AssetObj = TSoftObjectPtr<UObject>(FSoftObjectPath(AssetPath)).LoadSynchronous();
	if (!AssetObj)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("ASSET_NOT_FOUND"), FString::Printf(TEXT("Could not load asset '%s'."), *AssetPath));
		return false;
	}

	FString PackagePath = FPackageName::GetLongPackagePath(AssetPath);

	TArray<FAssetRenameData> AssetsToRename;
	FAssetRenameData RenameData;
	RenameData.Asset = AssetObj;
	RenameData.NewPackagePath = PackagePath;
	RenameData.NewName = NewName;
	AssetsToRename.Add(RenameData);

	bool bSuccess = AssetToolsModule.Get().RenameAssets(AssetsToRename);
	if (!bSuccess)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("RENAME_FAILED"), FString::Printf(TEXT("Failed to rename asset '%s' to '%s'."), *AssetPath, *NewName));
		return false;
	}

	FString NewAssetPath = PackagePath + TEXT("/") + NewName;

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("old_path"), AssetPath);
	ResultObj->SetStringField(TEXT("new_path"), NewAssetPath);
	ResultObj->SetStringField(TEXT("new_name"), NewName);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// delete_asset
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_DeleteAsset::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Asset path to delete (e.g. /Game/Weapons/BP_OldSword)."));
	Props->SetObjectField(TEXT("asset_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("asset_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_DeleteAsset::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString AssetPath = InArgs->GetStringField(TEXT("asset_path"));

	UObject* AssetObj = TSoftObjectPtr<UObject>(FSoftObjectPath(AssetPath)).LoadSynchronous();
	if (!AssetObj)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("ASSET_NOT_FOUND"), FString::Printf(TEXT("Could not find asset '%s' to delete."), *AssetPath));
		return false;
	}

	TArray<UObject*> ObjectsToDelete;
	ObjectsToDelete.Add(AssetObj);

	int32 DeletedCount = ObjectTools::ForceDeleteObjects(ObjectsToDelete, false);
	if (DeletedCount == 0)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("DELETE_FAILED"), FString::Printf(TEXT("Failed to delete asset '%s'."), *AssetPath));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("deleted_path"), AssetPath);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// save_asset
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_SaveAsset::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Optional package path to save. If omitted, saves all dirty packages."));
	Props->SetObjectField(TEXT("asset_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);
	return Schema;
}

bool FUnrealAITool_SaveAsset::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	if (InArgs->HasField(TEXT("asset_path")) && !InArgs->GetStringField(TEXT("asset_path")).IsEmpty())
	{
		FString AssetPath = InArgs->GetStringField(TEXT("asset_path"));
		FString PackageName = FPackageName::ObjectPathToPackageName(AssetPath);
		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (!Package)
		{
			Package = LoadPackage(nullptr, *PackageName, LOAD_None);
		}

		if (!Package)
		{
			OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("PACKAGE_NOT_FOUND"), FString::Printf(TEXT("Package for asset '%s' could not be loaded."), *AssetPath));
			return false;
		}

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(Package);
		bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

		TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
		ResultObj->SetStringField(TEXT("saved_asset"), AssetPath);
		ResultObj->SetBoolField(TEXT("saved"), bSaved);

		OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
		return true;
	}
	else
	{
		bool bSaved = UEditorLoadingAndSavingUtils::SaveDirtyPackages(false, true);
		TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
		ResultObj->SetBoolField(TEXT("saved_all_dirty"), bSaved);

		OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
		return true;
	}
}
