// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Tools/BlueprintTools/UnrealAIBlueprintGraphTools.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"

// -----------------------------------------------------------------------------
// Helper: Resolve Pin Type from String
// -----------------------------------------------------------------------------
static FEdGraphPinType ResolvePinType(const FString& InTypeName, bool bIsArray = false)
{
	FEdGraphPinType PinType;
	PinType.ContainerType = bIsArray ? EPinContainerType::Array : EPinContainerType::None;

	if (InTypeName.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
	}
	else if (InTypeName.Equals(TEXT("Integer"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("int"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("int32"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
	}
	else if (InTypeName.Equals(TEXT("Int64"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int64;
	}
	else if (InTypeName.Equals(TEXT("Float"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("Double"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("real"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
	}
	else if (InTypeName.Equals(TEXT("String"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FString"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_String;
	}
	else if (InTypeName.Equals(TEXT("Text"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FText"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
	}
	else if (InTypeName.Equals(TEXT("Name"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FName"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
	}
	else if (InTypeName.Equals(TEXT("Vector"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FVector"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
	}
	else if (InTypeName.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FRotator"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
	}
	else if (InTypeName.Equals(TEXT("Transform"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("FTransform"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
	}
	else if (InTypeName.Equals(TEXT("Actor"), ESearchCase::IgnoreCase) || InTypeName.Equals(TEXT("AActor"), ESearchCase::IgnoreCase))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		PinType.PinSubCategoryObject = AActor::StaticClass();
	}
	else
	{
		// Try finding a struct or class
		UScriptStruct* FoundStruct = UClass::TryFindTypeSlow<UScriptStruct>(InTypeName);
		if (FoundStruct)
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			PinType.PinSubCategoryObject = FoundStruct;
		}
		else
		{
			UClass* FoundClass = UClass::TryFindTypeSlow<UClass>(InTypeName);
			if (FoundClass)
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
				PinType.PinSubCategoryObject = FoundClass;
			}
			else
			{
				// Default fallback to Float
				PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
				PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
			}
		}
	}

	return PinType;
}

// -----------------------------------------------------------------------------
// list_blueprint_graphs
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_ListBlueprintGraphs::GetParametersSchema() const
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

bool FUnrealAITool_ListBlueprintGraphs::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);

	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> GraphsList;

	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GObj = MakeShared<FJsonObject>();
			GObj->SetStringField(TEXT("name"), Graph->GetName());
			GObj->SetStringField(TEXT("type"), TEXT("Ubergraph/EventGraph"));
			GObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
			GraphsList.Add(MakeShared<FJsonValueObject>(GObj));
		}
	}

	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> GObj = MakeShared<FJsonObject>();
			GObj->SetStringField(TEXT("name"), Graph->GetName());
			GObj->SetStringField(TEXT("type"), TEXT("Function"));
			GObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
			GraphsList.Add(MakeShared<FJsonValueObject>(GObj));
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetArrayField(TEXT("graphs"), GraphsList);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// create_variable
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CreateVariable::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Variable name (e.g. Damage, Health, IsEquipped)."));
	Props->SetObjectField(TEXT("variable_name"), NameProp);

	TSharedPtr<FJsonObject> TypeProp = MakeShared<FJsonObject>();
	TypeProp->SetStringField(TEXT("type"), TEXT("string"));
	TypeProp->SetStringField(TEXT("description"), TEXT("Variable type: Boolean, Integer, Float, Double, String, Text, Vector, Rotator, Transform, Actor."));
	Props->SetObjectField(TEXT("variable_type"), TypeProp);

	TSharedPtr<FJsonObject> DefValProp = MakeShared<FJsonObject>();
	DefValProp->SetStringField(TEXT("type"), TEXT("string"));
	DefValProp->SetStringField(TEXT("description"), TEXT("Optional default value literal (e.g. '35.0', 'true', 'Excalibur')."));
	Props->SetObjectField(TEXT("default_value"), DefValProp);

	TSharedPtr<FJsonObject> CatProp = MakeShared<FJsonObject>();
	CatProp->SetStringField(TEXT("type"), TEXT("string"));
	CatProp->SetStringField(TEXT("description"), TEXT("Optional variable category name (e.g. Stats, Combat)."));
	Props->SetObjectField(TEXT("category"), CatProp);

	TSharedPtr<FJsonObject> ArrayProp = MakeShared<FJsonObject>();
	ArrayProp->SetStringField(TEXT("type"), TEXT("boolean"));
	ArrayProp->SetStringField(TEXT("description"), TEXT("Whether the variable is an array container (default false)."));
	Props->SetObjectField(TEXT("is_array"), ArrayProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("variable_name")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("variable_type")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CreateVariable::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString VarName = InArgs->GetStringField(TEXT("variable_name"));
	FString VarTypeStr = InArgs->GetStringField(TEXT("variable_type"));
	FString DefaultValue = InArgs->HasField(TEXT("default_value")) ? InArgs->GetStringField(TEXT("default_value")) : TEXT("");
	FString Category = InArgs->HasField(TEXT("category")) ? InArgs->GetStringField(TEXT("category")) : TEXT("Default");
	bool bIsArray = InArgs->HasField(TEXT("is_array")) && InArgs->GetBoolField(TEXT("is_array"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	FEdGraphPinType PinType = ResolvePinType(VarTypeStr, bIsArray);

	bool bAdded = FBlueprintEditorUtils::AddMemberVariable(BP, FName(*VarName), PinType, DefaultValue);
	if (!bAdded)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("VARIABLE_CREATION_FAILED"), FString::Printf(TEXT("Failed to add variable '%s' to Blueprint '%s'."), *VarName, *BPPath));
		return false;
	}

	if (!Category.IsEmpty())
	{
		FBlueprintEditorUtils::SetBlueprintVariableCategory(BP, FName(*VarName), nullptr, FText::FromString(Category));
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("variable_name"), VarName);
	ResultObj->SetStringField(TEXT("variable_type"), VarTypeStr);
	ResultObj->SetStringField(TEXT("default_value"), DefaultValue);
	ResultObj->SetStringField(TEXT("category"), Category);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// modify_variable
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_ModifyVariable::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Current variable name."));
	Props->SetObjectField(TEXT("variable_name"), NameProp);

	TSharedPtr<FJsonObject> NewNameProp = MakeShared<FJsonObject>();
	NewNameProp->SetStringField(TEXT("type"), TEXT("string"));
	NewNameProp->SetStringField(TEXT("description"), TEXT("Optional new variable name."));
	Props->SetObjectField(TEXT("new_variable_name"), NewNameProp);

	TSharedPtr<FJsonObject> TypeProp = MakeShared<FJsonObject>();
	TypeProp->SetStringField(TEXT("type"), TEXT("string"));
	TypeProp->SetStringField(TEXT("description"), TEXT("Optional new type."));
	Props->SetObjectField(TEXT("variable_type"), TypeProp);

	TSharedPtr<FJsonObject> DefValProp = MakeShared<FJsonObject>();
	DefValProp->SetStringField(TEXT("type"), TEXT("string"));
	DefValProp->SetStringField(TEXT("description"), TEXT("Optional new default value literal."));
	Props->SetObjectField(TEXT("default_value"), DefValProp);

	TSharedPtr<FJsonObject> CatProp = MakeShared<FJsonObject>();
	CatProp->SetStringField(TEXT("type"), TEXT("string"));
	CatProp->SetStringField(TEXT("description"), TEXT("Optional new category."));
	Props->SetObjectField(TEXT("category"), CatProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("variable_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_ModifyVariable::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString VarName = InArgs->GetStringField(TEXT("variable_name"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	int32 VarIdx = FBlueprintEditorUtils::FindNewVariableIndex(BP, FName(*VarName));
	if (VarIdx == INDEX_NONE)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("VARIABLE_NOT_FOUND"), FString::Printf(TEXT("Variable '%s' not found on Blueprint '%s'."), *VarName, *BPPath));
		return false;
	}

	if (InArgs->HasField(TEXT("variable_type")))
	{
		FString VarTypeStr = InArgs->GetStringField(TEXT("variable_type"));
		FEdGraphPinType NewPinType = ResolvePinType(VarTypeStr);
		FBlueprintEditorUtils::ChangeMemberVariableType(BP, FName(*VarName), NewPinType);
	}

	if (InArgs->HasField(TEXT("default_value")))
	{
		FString NewDefVal = InArgs->GetStringField(TEXT("default_value"));
		BP->NewVariables[VarIdx].DefaultValue = NewDefVal;
	}

	if (InArgs->HasField(TEXT("category")))
	{
		FString NewCat = InArgs->GetStringField(TEXT("category"));
		FBlueprintEditorUtils::SetBlueprintVariableCategory(BP, FName(*VarName), nullptr, FText::FromString(NewCat));
	}

	if (InArgs->HasField(TEXT("new_variable_name")))
	{
		FString NewVarName = InArgs->GetStringField(TEXT("new_variable_name"));
		FBlueprintEditorUtils::RenameMemberVariable(BP, FName(*VarName), FName(*NewVarName));
		VarName = NewVarName;
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("variable_name"), VarName);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// delete_variable
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_DeleteVariable::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Variable name to delete."));
	Props->SetObjectField(TEXT("variable_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("variable_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_DeleteVariable::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString VarName = InArgs->GetStringField(TEXT("variable_name"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	FBlueprintEditorUtils::RemoveMemberVariable(BP, FName(*VarName));
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("deleted_variable"), VarName);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// create_function
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CreateFunction::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Function name (e.g. CalculateDamage, ApplyBuff)."));
	Props->SetObjectField(TEXT("function_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("function_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CreateFunction::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString FuncName = InArgs->GetStringField(TEXT("function_name"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(
		BP,
		FName(*FuncName),
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (!NewGraph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("FUNCTION_CREATION_FAILED"), FString::Printf(TEXT("Failed to create function graph '%s'."), *FuncName));
		return false;
	}

	FBlueprintEditorUtils::AddFunctionGraph<UClass>(BP, NewGraph, false, (UClass*)nullptr);
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("function_name"), FuncName);
	ResultObj->SetStringField(TEXT("graph_name"), NewGraph->GetName());

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// delete_function
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_DeleteFunction::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Function name to delete."));
	Props->SetObjectField(TEXT("function_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("function_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_DeleteFunction::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString FuncName = InArgs->GetStringField(TEXT("function_name"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* FoundGraph = nullptr;
	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph && Graph->GetName().Equals(FuncName, ESearchCase::IgnoreCase))
		{
			FoundGraph = Graph;
			break;
		}
	}

	if (!FoundGraph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("FUNCTION_NOT_FOUND"), FString::Printf(TEXT("Function '%s' not found on Blueprint '%s'."), *FuncName, *BPPath));
		return false;
	}

	FBlueprintEditorUtils::RemoveGraph(BP, FoundGraph);
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("deleted_function"), FuncName);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}
