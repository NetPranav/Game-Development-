// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Tools/BlueprintTools/UnrealAIBlueprintTools.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "FileHelpers.h"
#include "Misc/PackageName.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"

// -----------------------------------------------------------------------------
// Helper: Resolve Parent Class
// -----------------------------------------------------------------------------
static UClass* ResolveParentClass(const FString& InClassName)
{
	if (InClassName.IsEmpty() || InClassName.Equals(TEXT("Actor"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("AActor"), ESearchCase::IgnoreCase))
	{
		return AActor::StaticClass();
	}
	if (InClassName.Equals(TEXT("Character"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("ACharacter"), ESearchCase::IgnoreCase))
	{
		return ACharacter::StaticClass();
	}
	if (InClassName.Equals(TEXT("Pawn"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("APawn"), ESearchCase::IgnoreCase))
	{
		return APawn::StaticClass();
	}
	if (InClassName.Equals(TEXT("ActorComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("UActorComponent"), ESearchCase::IgnoreCase))
	{
		return UActorComponent::StaticClass();
	}

	// Try finding by path or short name
	UClass* FoundClass = UClass::TryFindTypeSlow<UClass>(InClassName);
	if (FoundClass)
	{
		return FoundClass;
	}

	FoundClass = LoadObject<UClass>(nullptr, *InClassName);
	if (FoundClass)
	{
		return FoundClass;
	}

	// Fallback to AActor
	return AActor::StaticClass();
}

// -----------------------------------------------------------------------------
// create_blueprint
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CreateBlueprint::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Blueprint asset name (e.g. BP_Sword)."));
	Props->SetObjectField(TEXT("name"), NameProp);

	TSharedPtr<FJsonObject> FolderProp = MakeShared<FJsonObject>();
	FolderProp->SetStringField(TEXT("type"), TEXT("string"));
	FolderProp->SetStringField(TEXT("description"), TEXT("Target folder path under /Game (e.g. /Game/Weapons)."));
	Props->SetObjectField(TEXT("folder"), FolderProp);

	TSharedPtr<FJsonObject> ParentClassProp = MakeShared<FJsonObject>();
	ParentClassProp->SetStringField(TEXT("type"), TEXT("string"));
	ParentClassProp->SetStringField(TEXT("description"), TEXT("Parent class (default 'Actor', can be 'Character', 'Pawn', 'ActorComponent', or custom class)."));
	Props->SetObjectField(TEXT("parent_class"), ParentClassProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("name")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("folder")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CreateBlueprint::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString AssetName = InArgs->GetStringField(TEXT("name"));
	FString FolderPath = InArgs->GetStringField(TEXT("folder"));
	FString ParentClassName = InArgs->HasField(TEXT("parent_class")) ? InArgs->GetStringField(TEXT("parent_class")) : TEXT("Actor");

	if (!FolderPath.StartsWith(TEXT("/Game")))
	{
		FolderPath = TEXT("/Game/") + FolderPath;
	}

	FString PackagePath = FolderPath + TEXT("/") + AssetName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("PACKAGE_CREATION_FAILED"), FString::Printf(TEXT("Failed to create package '%s'."), *PackagePath));
		return false;
	}

	UClass* ParentClass = ResolveParentClass(ParentClassName);

	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName(TEXT("UnrealAIAgent"))
	);

	if (!NewBP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_CREATION_FAILED"), FString::Printf(TEXT("Failed to create Blueprint '%s' with parent '%s'."), *AssetName, *ParentClass->GetName()));
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(NewBP);
	Package->MarkPackageDirty();

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_name"), AssetName);
	ResultObj->SetStringField(TEXT("blueprint_path"), PackagePath);
	ResultObj->SetStringField(TEXT("parent_class"), ParentClass->GetName());

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// open_blueprint
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_OpenBlueprint::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_OpenBlueprint::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);

	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	if (GEditor)
	{
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		if (AssetEditorSubsystem)
		{
			AssetEditorSubsystem->OpenEditorForAsset(BP);
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetBoolField(TEXT("opened"), true);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// inspect_blueprint
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_InspectBlueprint::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_InspectBlueprint::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);

	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("name"), BP->GetName());
	ResultObj->SetStringField(TEXT("parent_class"), BP->ParentClass ? BP->ParentClass->GetName() : TEXT("None"));

	// Variables
	TArray<TSharedPtr<FJsonValue>> VarsArray;
	for (const FBPVariableDescription& VarDesc : BP->NewVariables)
	{
		TSharedPtr<FJsonObject> VarObj = MakeShared<FJsonObject>();
		VarObj->SetStringField(TEXT("name"), VarDesc.VarName.ToString());
		VarObj->SetStringField(TEXT("type"), VarDesc.VarType.PinCategory.ToString());
		VarObj->SetStringField(TEXT("default_value"), VarDesc.DefaultValue);
		VarObj->SetStringField(TEXT("category"), VarDesc.Category.ToString());
		VarObj->SetBoolField(TEXT("is_array"), VarDesc.VarType.IsArray());
		VarsArray.Add(MakeShared<FJsonValueObject>(VarObj));
	}
	ResultObj->SetArrayField(TEXT("variables"), VarsArray);

	// Graphs
	TArray<TSharedPtr<FJsonValue>> GraphsArray;
	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GraphObj = MakeShared<FJsonObject>();
			GraphObj->SetStringField(TEXT("name"), Graph->GetName());
			GraphObj->SetStringField(TEXT("type"), TEXT("EventGraph"));
			GraphObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
			GraphsArray.Add(MakeShared<FJsonValueObject>(GraphObj));
		}
	}
	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GraphObj = MakeShared<FJsonObject>();
			GraphObj->SetStringField(TEXT("name"), Graph->GetName());
			GraphObj->SetStringField(TEXT("type"), TEXT("Function"));
			GraphObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
			GraphsArray.Add(MakeShared<FJsonValueObject>(GraphObj));
		}
	}
	ResultObj->SetArrayField(TEXT("graphs"), GraphsArray);

	// Components
	TArray<TSharedPtr<FJsonValue>> CompsArray;
	if (BP->SimpleConstructionScript)
	{
		const TArray<USCS_Node*>& SCSNodes = BP->SimpleConstructionScript->GetAllNodes();
		for (USCS_Node* SCSNode : SCSNodes)
		{
			if (SCSNode && SCSNode->ComponentClass)
			{
				TSharedPtr<FJsonObject> CompObj = MakeShared<FJsonObject>();
				CompObj->SetStringField(TEXT("name"), SCSNode->GetVariableName().ToString());
				CompObj->SetStringField(TEXT("class"), SCSNode->ComponentClass->GetName());
				if (SCSNode->ParentComponentOrVariableName != NAME_None)
				{
					CompObj->SetStringField(TEXT("parent"), SCSNode->ParentComponentOrVariableName.ToString());
				}
				CompsArray.Add(MakeShared<FJsonValueObject>(CompObj));
			}
		}
	}
	ResultObj->SetArrayField(TEXT("components"), CompsArray);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// save_blueprint
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_SaveBlueprint::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_SaveBlueprint::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);

	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UPackage* Package = BP->GetOutermost();
	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);

	bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetBoolField(TEXT("saved"), bSaved);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}
