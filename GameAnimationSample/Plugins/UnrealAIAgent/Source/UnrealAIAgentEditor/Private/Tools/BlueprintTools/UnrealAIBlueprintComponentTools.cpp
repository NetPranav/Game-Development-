// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Tools/BlueprintTools/UnrealAIBlueprintComponentTools.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// -----------------------------------------------------------------------------
// Helper: Resolve Component Class
// -----------------------------------------------------------------------------
static UClass* ResolveComponentClass(const FString& InClassName)
{
	if (InClassName.Equals(TEXT("StaticMeshComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("StaticMesh"), ESearchCase::IgnoreCase))
	{
		return UStaticMeshComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("SphereComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("Sphere"), ESearchCase::IgnoreCase))
	{
		return USphereComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("BoxComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("Box"), ESearchCase::IgnoreCase))
	{
		return UBoxComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("CapsuleComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("Capsule"), ESearchCase::IgnoreCase))
	{
		return UCapsuleComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("AudioComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("Audio"), ESearchCase::IgnoreCase))
	{
		return UAudioComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("PointLightComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("PointLight"), ESearchCase::IgnoreCase))
	{
		return UPointLightComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("SpotLightComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("SpotLight"), ESearchCase::IgnoreCase))
	{
		return USpotLightComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("CameraComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("Camera"), ESearchCase::IgnoreCase))
	{
		return UCameraComponent::StaticClass();
	}
	if (InClassName.Equals(TEXT("SpringArmComponent"), ESearchCase::IgnoreCase) || InClassName.Equals(TEXT("SpringArm"), ESearchCase::IgnoreCase))
	{
		return USpringArmComponent::StaticClass();
	}

	// Try finding by path or class name
	UClass* FoundClass = UClass::TryFindTypeSlow<UClass>(InClassName);
	if (FoundClass && FoundClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return FoundClass;
	}

	return UStaticMeshComponent::StaticClass();
}

// -----------------------------------------------------------------------------
// add_component
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_AddComponent::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> ClassProp = MakeShared<FJsonObject>();
	ClassProp->SetStringField(TEXT("type"), TEXT("string"));
	ClassProp->SetStringField(TEXT("description"), TEXT("Component class: StaticMeshComponent, BoxComponent, SphereComponent, CapsuleComponent, AudioComponent, CameraComponent, SpringArmComponent, PointLightComponent."));
	Props->SetObjectField(TEXT("component_class"), ClassProp);

	TSharedPtr<FJsonObject> NameProp = MakeShared<FJsonObject>();
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("Unique variable name for the component (e.g. SwordMesh, InteractionBox)."));
	Props->SetObjectField(TEXT("component_name"), NameProp);

	TSharedPtr<FJsonObject> AttachProp = MakeShared<FJsonObject>();
	AttachProp->SetStringField(TEXT("type"), TEXT("string"));
	AttachProp->SetStringField(TEXT("description"), TEXT("Optional name of parent component to attach to (default: RootComponent)."));
	Props->SetObjectField(TEXT("attach_to"), AttachProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("component_class")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("component_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_AddComponent::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString CompClassName = InArgs->GetStringField(TEXT("component_class"));
	FString CompName = InArgs->GetStringField(TEXT("component_name"));
	FString AttachTo = InArgs->HasField(TEXT("attach_to")) ? InArgs->GetStringField(TEXT("attach_to")) : TEXT("");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	if (!BP->SimpleConstructionScript)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("SCS_NOT_FOUND"), TEXT("Blueprint does not have a SimpleConstructionScript."));
		return false;
	}

	UClass* CompClass = ResolveComponentClass(CompClassName);
	if (!CompClass)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("INVALID_COMPONENT_CLASS"), FString::Printf(TEXT("Component class '%s' could not be resolved."), *CompClassName));
		return false;
	}

	USCS_Node* NewSCSNode = BP->SimpleConstructionScript->CreateNode(CompClass, FName(*CompName));
	if (!NewSCSNode)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("COMPONENT_CREATION_FAILED"), FString::Printf(TEXT("Failed to create component node for '%s'."), *CompName));
		return false;
	}

	USCS_Node* ParentNode = nullptr;
	if (!AttachTo.IsEmpty())
	{
		ParentNode = BP->SimpleConstructionScript->FindSCSNode(FName(*AttachTo));
	}

	if (ParentNode)
	{
		ParentNode->AddChildNode(NewSCSNode);
	}
	else
	{
		BP->SimpleConstructionScript->AddNode(NewSCSNode);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("component_name"), CompName);
	ResultObj->SetStringField(TEXT("component_class"), CompClass->GetName());
	if (ParentNode)
	{
		ResultObj->SetStringField(TEXT("attached_to"), AttachTo);
	}

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// set_component_property
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_SetComponentProperty::GetParametersSchema() const
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
	NameProp->SetStringField(TEXT("description"), TEXT("Component variable name."));
	Props->SetObjectField(TEXT("component_name"), NameProp);

	TSharedPtr<FJsonObject> MeshProp = MakeShared<FJsonObject>();
	MeshProp->SetStringField(TEXT("type"), TEXT("string"));
	MeshProp->SetStringField(TEXT("description"), TEXT("StaticMesh asset path (e.g. /Game/Weapons/SM_Sword or /Engine/BasicShapes/Cube)."));
	Props->SetObjectField(TEXT("static_mesh_path"), MeshProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("component_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_SetComponentProperty::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString CompName = InArgs->GetStringField(TEXT("component_name"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	if (!BP->SimpleConstructionScript)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("SCS_NOT_FOUND"), TEXT("Blueprint does not have a SimpleConstructionScript."));
		return false;
	}

	USCS_Node* SCSNode = BP->SimpleConstructionScript->FindSCSNode(FName(*CompName));
	if (!SCSNode)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("COMPONENT_NOT_FOUND"), FString::Printf(TEXT("Component '%s' not found in Blueprint '%s'."), *CompName, *BPPath));
		return false;
	}

	UActorComponent* CompTemplate = SCSNode->ComponentTemplate;
	if (!CompTemplate)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("TEMPLATE_NOT_FOUND"), FString::Printf(TEXT("Component template for '%s' is null."), *CompName));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("component_name"), CompName);

	if (InArgs->HasField(TEXT("static_mesh_path")))
	{
		FString MeshPath = InArgs->GetStringField(TEXT("static_mesh_path"));
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(CompTemplate);
		if (!SMC)
		{
			OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NOT_STATIC_MESH_COMPONENT"), FString::Printf(TEXT("Component '%s' is not a StaticMeshComponent."), *CompName));
			return false;
		}

		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
		if (!Mesh)
		{
			OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("MESH_NOT_FOUND"), FString::Printf(TEXT("Could not load StaticMesh at '%s'."), *MeshPath));
			return false;
		}

		SMC->SetStaticMesh(Mesh);
		ResultObj->SetStringField(TEXT("assigned_mesh"), MeshPath);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}
