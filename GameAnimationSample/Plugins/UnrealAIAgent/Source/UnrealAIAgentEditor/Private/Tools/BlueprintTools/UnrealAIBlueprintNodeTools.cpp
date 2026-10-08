// Copyright (c) 2026 Antigravity. All Rights Reserved.

#include "Tools/BlueprintTools/UnrealAIBlueprintNodeTools.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "EdGraphSchema_K2.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"

// -----------------------------------------------------------------------------
// Helper: Find Graph by Name in Blueprint
// -----------------------------------------------------------------------------
static UEdGraph* FindBlueprintGraph(UBlueprint* BP, const FString& InGraphName)
{
	if (!BP)
	{
		return nullptr;
	}

	if (InGraphName.IsEmpty() || InGraphName.Equals(TEXT("EventGraph"), ESearchCase::IgnoreCase))
	{
		if (BP->UbergraphPages.Num() > 0)
		{
			return BP->UbergraphPages[0];
		}
	}

	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (Graph && Graph->GetName().Equals(InGraphName, ESearchCase::IgnoreCase))
		{
			return Graph;
		}
	}

	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph && Graph->GetName().Equals(InGraphName, ESearchCase::IgnoreCase))
		{
			return Graph;
		}
	}

	for (UEdGraph* Graph : BP->MacroGraphs)
	{
		if (Graph && Graph->GetName().Equals(InGraphName, ESearchCase::IgnoreCase))
		{
			return Graph;
		}
	}

	return nullptr;
}

// -----------------------------------------------------------------------------
// Helper: Find Node by GUID or Name
// -----------------------------------------------------------------------------
static UEdGraphNode* FindGraphNode(UEdGraph* Graph, const FString& NodeIdOrName)
{
	if (!Graph)
	{
		return nullptr;
	}

	FGuid TargetGuid;
	const bool bIsGuid = FGuid::Parse(NodeIdOrName, TargetGuid);

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (!Node)
		{
			continue;
		}

		if (bIsGuid && Node->NodeGuid == TargetGuid)
		{
			return Node;
		}

		if (Node->GetName().Equals(NodeIdOrName, ESearchCase::IgnoreCase) ||
			Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString().Equals(NodeIdOrName, ESearchCase::IgnoreCase))
		{
			return Node;
		}
	}

	return nullptr;
}

// -----------------------------------------------------------------------------
// Helper: Find Pin on Node
// -----------------------------------------------------------------------------
static UEdGraphPin* FindNodePin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction = EGPD_MAX)
{
	if (!Node)
	{
		return nullptr;
	}

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (!Pin)
		{
			continue;
		}

		if (Direction != EGPD_MAX && Pin->Direction != Direction)
		{
			continue;
		}

		if (Pin->PinName.ToString().Equals(PinName, ESearchCase::IgnoreCase))
		{
			return Pin;
		}
	}

	// Secondary check: partial match if exact name not found (e.g. Return Value vs ReturnValue)
	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (!Pin)
		{
			continue;
		}

		if (Direction != EGPD_MAX && Pin->Direction != Direction)
		{
			continue;
		}

		FString SanitizedPin = Pin->PinName.ToString().Replace(TEXT(" "), TEXT(""));
		FString SanitizedTarget = PinName.Replace(TEXT(" "), TEXT(""));
		if (SanitizedPin.Equals(SanitizedTarget, ESearchCase::IgnoreCase))
		{
			return Pin;
		}
	}

	return nullptr;
}

// -----------------------------------------------------------------------------
// get_graph_nodes
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetGraphNodes::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path (e.g. /Game/Weapons/BP_Sword)."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Graph name (e.g. EventGraph, or custom function name). Default: EventGraph."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_GetGraphNodes::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found in Blueprint '%s'."), *GraphName, *BPPath));
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> NodesArray;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (!Node)
		{
			continue;
		}

		TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
		NodeObj->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
		NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
		NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
		NodeObj->SetNumberField(TEXT("pos_x"), Node->NodePosX);
		NodeObj->SetNumberField(TEXT("pos_y"), Node->NodePosY);

		NodesArray.Add(MakeShared<FJsonValueObject>(NodeObj));
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("graph_name"), Graph->GetName());
	ResultObj->SetNumberField(TEXT("node_count"), NodesArray.Num());
	ResultObj->SetArrayField(TEXT("nodes"), NodesArray);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// get_node_info
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetNodeInfo::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_GetNodeInfo::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found in graph '%s'."), *NodeId, *GraphName));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
	ResultObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
	ResultObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
	ResultObj->SetNumberField(TEXT("pos_x"), Node->NodePosX);
	ResultObj->SetNumberField(TEXT("pos_y"), Node->NodePosY);
	ResultObj->SetStringField(TEXT("node_comment"), Node->NodeComment);

	// Pins
	TArray<TSharedPtr<FJsonValue>> PinsArray;
	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (!Pin)
		{
			continue;
		}

		TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
		PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
		PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
		PinObj->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
		PinObj->SetStringField(TEXT("sub_category"), Pin->PinType.PinSubCategory.ToString());
		PinObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);

		TArray<TSharedPtr<FJsonValue>> LinkedArray;
		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			if (LinkedPin && LinkedPin->GetOwningNode())
			{
				TSharedPtr<FJsonObject> LinkObj = MakeShared<FJsonObject>();
				LinkObj->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
				LinkObj->SetStringField(TEXT("node_title"), LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
				LinkObj->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
				LinkedArray.Add(MakeShared<FJsonValueObject>(LinkObj));
			}
		}
		PinObj->SetArrayField(TEXT("linked_to"), LinkedArray);

		PinsArray.Add(MakeShared<FJsonValueObject>(PinObj));
	}
	ResultObj->SetArrayField(TEXT("pins"), PinsArray);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// get_node_pins
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetNodePins::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_GetNodePins::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	return FUnrealAITool_GetNodeInfo().Execute(InArgs, OutResult);
}

// -----------------------------------------------------------------------------
// find_node
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_FindNode::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> QueryProp = MakeShared<FJsonObject>();
	QueryProp->SetStringField(TEXT("type"), TEXT("string"));
	QueryProp->SetStringField(TEXT("description"), TEXT("Search term (e.g. 'Branch', 'BeginPlay', 'Damage', 'PrintString')."));
	Props->SetObjectField(TEXT("search_query"), QueryProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("search_query")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_FindNode::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString Query = InArgs->GetStringField(TEXT("search_query"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> MatchedNodes;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (!Node)
		{
			continue;
		}

		FString Title = Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
		FString ClassName = Node->GetClass()->GetName();

		if (Title.Contains(Query, ESearchCase::IgnoreCase) || ClassName.Contains(Query, ESearchCase::IgnoreCase))
		{
			TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
			NodeObj->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
			NodeObj->SetStringField(TEXT("title"), Title);
			NodeObj->SetStringField(TEXT("class"), ClassName);
			NodeObj->SetNumberField(TEXT("pos_x"), Node->NodePosX);
			NodeObj->SetNumberField(TEXT("pos_y"), Node->NodePosY);
			MatchedNodes.Add(MakeShared<FJsonValueObject>(NodeObj));
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetNumberField(TEXT("total_found"), MatchedNodes.Num());
	ResultObj->SetArrayField(TEXT("nodes"), MatchedNodes);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// find_pin
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_FindPin::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> PinProp = MakeShared<FJsonObject>();
	PinProp->SetStringField(TEXT("type"), TEXT("string"));
	PinProp->SetStringField(TEXT("description"), TEXT("Pin name to find (e.g. 'Condition', 'Then', 'Damage', 'ReturnValue')."));
	Props->SetObjectField(TEXT("pin_name"), PinProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("pin_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_FindPin::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	FString PinName = InArgs->GetStringField(TEXT("pin_name"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found."), *NodeId));
		return false;
	}

	UEdGraphPin* FoundPin = FindNodePin(Node, PinName);
	if (!FoundPin)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("PIN_NOT_FOUND"), FString::Printf(TEXT("Pin '%s' not found on node '%s'."), *PinName, *NodeId));
		return false;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("pin_name"), FoundPin->PinName.ToString());
	ResultObj->SetStringField(TEXT("direction"), FoundPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
	ResultObj->SetStringField(TEXT("category"), FoundPin->PinType.PinCategory.ToString());
	ResultObj->SetStringField(TEXT("default_value"), FoundPin->DefaultValue);
	ResultObj->SetNumberField(TEXT("link_count"), FoundPin->LinkedTo.Num());

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// create_node
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CreateNode::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> TypeProp = MakeShared<FJsonObject>();
	TypeProp->SetStringField(TEXT("type"), TEXT("string"));
	TypeProp->SetStringField(TEXT("description"), TEXT("Node Type: 'CallFunction', 'CustomEvent', 'Event', 'VariableGet', 'VariableSet', 'Branch' / 'IfThenElse', 'Sequence', 'DynamicCast', 'PrintString', 'Math'."));
	Props->SetObjectField(TEXT("node_type"), TypeProp);

	TSharedPtr<FJsonObject> MemberProp = MakeShared<FJsonObject>();
	MemberProp->SetStringField(TEXT("type"), TEXT("string"));
	MemberProp->SetStringField(TEXT("description"), TEXT("Function name, Event name, Variable name, or Operator name depending on node_type."));
	Props->SetObjectField(TEXT("member_name"), MemberProp);

	TSharedPtr<FJsonObject> PosXProp = MakeShared<FJsonObject>();
	PosXProp->SetStringField(TEXT("type"), TEXT("number"));
	PosXProp->SetStringField(TEXT("description"), TEXT("X position in graph (default: 0)."));
	Props->SetObjectField(TEXT("pos_x"), PosXProp);

	TSharedPtr<FJsonObject> PosYProp = MakeShared<FJsonObject>();
	PosYProp->SetStringField(TEXT("type"), TEXT("number"));
	PosYProp->SetStringField(TEXT("description"), TEXT("Y position in graph (default: 0)."));
	Props->SetObjectField(TEXT("pos_y"), PosYProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_type")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CreateNode::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeType = InArgs->GetStringField(TEXT("node_type"));
	FString MemberName = InArgs->HasField(TEXT("member_name")) ? InArgs->GetStringField(TEXT("member_name")) : TEXT("");
	int32 PosX = InArgs->HasField(TEXT("pos_x")) ? InArgs->GetIntegerField(TEXT("pos_x")) : 0;
	int32 PosY = InArgs->HasField(TEXT("pos_y")) ? InArgs->GetIntegerField(TEXT("pos_y")) : 0;
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* CreatedNode = nullptr;

	if (NodeType.Equals(TEXT("Branch"), ESearchCase::IgnoreCase) || NodeType.Equals(TEXT("IfThenElse"), ESearchCase::IgnoreCase))
	{
		UK2Node_IfThenElse* BranchNode = NewObject<UK2Node_IfThenElse>(Graph);
		BranchNode->CreateNewGuid();
		BranchNode->NodePosX = PosX;
		BranchNode->NodePosY = PosY;
		Graph->AddNode(BranchNode);
		BranchNode->AllocateDefaultPins();
		CreatedNode = BranchNode;
	}
	else if (NodeType.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase) || NodeType.Equals(TEXT("ExecutionSequence"), ESearchCase::IgnoreCase))
	{
		UK2Node_ExecutionSequence* SeqNode = NewObject<UK2Node_ExecutionSequence>(Graph);
		SeqNode->CreateNewGuid();
		SeqNode->NodePosX = PosX;
		SeqNode->NodePosY = PosY;
		Graph->AddNode(SeqNode);
		SeqNode->AllocateDefaultPins();
		CreatedNode = SeqNode;
	}
	else if (NodeType.Equals(TEXT("CustomEvent"), ESearchCase::IgnoreCase))
	{
		UK2Node_CustomEvent* EventNode = NewObject<UK2Node_CustomEvent>(Graph);
		EventNode->CreateNewGuid();
		EventNode->NodePosX = PosX;
		EventNode->NodePosY = PosY;
		EventNode->CustomFunctionName = FName(MemberName.IsEmpty() ? TEXT("CustomEvent") : *MemberName);
		Graph->AddNode(EventNode);
		EventNode->AllocateDefaultPins();
		CreatedNode = EventNode;
	}
	else if (NodeType.Equals(TEXT("Event"), ESearchCase::IgnoreCase))
	{
		UK2Node_Event* EventNode = NewObject<UK2Node_Event>(Graph);
		EventNode->CreateNewGuid();
		EventNode->NodePosX = PosX;
		EventNode->NodePosY = PosY;
		EventNode->EventReference.SetExternalMember(FName(*MemberName), AActor::StaticClass());
		Graph->AddNode(EventNode);
		EventNode->AllocateDefaultPins();
		CreatedNode = EventNode;
	}
	else if (NodeType.Equals(TEXT("VariableGet"), ESearchCase::IgnoreCase) || NodeType.Equals(TEXT("Get"), ESearchCase::IgnoreCase))
	{
		UK2Node_VariableGet* VarGetNode = NewObject<UK2Node_VariableGet>(Graph);
		VarGetNode->CreateNewGuid();
		VarGetNode->NodePosX = PosX;
		VarGetNode->NodePosY = PosY;
		VarGetNode->VariableReference.SetSelfMember(FName(*MemberName));
		Graph->AddNode(VarGetNode);
		VarGetNode->AllocateDefaultPins();
		CreatedNode = VarGetNode;
	}
	else if (NodeType.Equals(TEXT("VariableSet"), ESearchCase::IgnoreCase) || NodeType.Equals(TEXT("Set"), ESearchCase::IgnoreCase))
	{
		UK2Node_VariableSet* VarSetNode = NewObject<UK2Node_VariableSet>(Graph);
		VarSetNode->CreateNewGuid();
		VarSetNode->NodePosX = PosX;
		VarSetNode->NodePosY = PosY;
		VarSetNode->VariableReference.SetSelfMember(FName(*MemberName));
		Graph->AddNode(VarSetNode);
		VarSetNode->AllocateDefaultPins();
		CreatedNode = VarSetNode;
	}
	else if (NodeType.Equals(TEXT("PrintString"), ESearchCase::IgnoreCase))
	{
		UK2Node_CallFunction* CallFunc = NewObject<UK2Node_CallFunction>(Graph);
		CallFunc->CreateNewGuid();
		CallFunc->NodePosX = PosX;
		CallFunc->NodePosY = PosY;
		CallFunc->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintString), UKismetSystemLibrary::StaticClass());
		Graph->AddNode(CallFunc);
		CallFunc->AllocateDefaultPins();
		CreatedNode = CallFunc;
	}
	else if (NodeType.Equals(TEXT("CallFunction"), ESearchCase::IgnoreCase))
	{
		UK2Node_CallFunction* CallFunc = NewObject<UK2Node_CallFunction>(Graph);
		CallFunc->CreateNewGuid();
		CallFunc->NodePosX = PosX;
		CallFunc->NodePosY = PosY;

		// Check math/system library functions first
		UClass* TargetFunctionClass = UKismetSystemLibrary::StaticClass();
		UFunction* TargetFunc = UKismetSystemLibrary::StaticClass()->FindFunctionByName(FName(*MemberName));
		if (!TargetFunc)
		{
			TargetFunc = UKismetMathLibrary::StaticClass()->FindFunctionByName(FName(*MemberName));
			if (TargetFunc)
			{
				TargetFunctionClass = UKismetMathLibrary::StaticClass();
			}
		}
		if (!TargetFunc)
		{
			TargetFunc = UGameplayStatics::StaticClass()->FindFunctionByName(FName(*MemberName));
			if (TargetFunc)
			{
				TargetFunctionClass = UGameplayStatics::StaticClass();
			}
		}
		if (!TargetFunc && BP->ParentClass)
		{
			TargetFunc = BP->ParentClass->FindFunctionByName(FName(*MemberName));
			if (TargetFunc)
			{
				TargetFunctionClass = BP->ParentClass;
			}
		}

		if (TargetFunc)
		{
			CallFunc->FunctionReference.SetExternalMember(FName(*MemberName), TargetFunctionClass);
		}
		else
		{
			CallFunc->FunctionReference.SetExternalMember(FName(*MemberName), UKismetSystemLibrary::StaticClass());
		}

		Graph->AddNode(CallFunc);
		CallFunc->AllocateDefaultPins();
		CreatedNode = CallFunc;
	}
	else
	{
		// Default to CallFunction
		UK2Node_CallFunction* CallFunc = NewObject<UK2Node_CallFunction>(Graph);
		CallFunc->CreateNewGuid();
		CallFunc->NodePosX = PosX;
		CallFunc->NodePosY = PosY;
		CallFunc->FunctionReference.SetExternalMember(FName(*MemberName), UKismetSystemLibrary::StaticClass());
		Graph->AddNode(CallFunc);
		CallFunc->AllocateDefaultPins();
		CreatedNode = CallFunc;
	}

	if (!CreatedNode)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_CREATION_FAILED"), FString::Printf(TEXT("Failed to instantiate node of type '%s'."), *NodeType));
		return false;
	}

	CreatedNode->ReconstructNode();
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("node_id"), CreatedNode->NodeGuid.ToString());
	ResultObj->SetStringField(TEXT("title"), CreatedNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
	ResultObj->SetStringField(TEXT("node_type"), NodeType);
	ResultObj->SetNumberField(TEXT("pos_x"), PosX);
	ResultObj->SetNumberField(TEXT("pos_y"), PosY);

	// Include available pins so the model can immediately connect them
	TArray<TSharedPtr<FJsonValue>> PinsArray;
	for (UEdGraphPin* Pin : CreatedNode->Pins)
	{
		if (Pin)
		{
			TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
			PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
			PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
			PinObj->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
			PinsArray.Add(MakeShared<FJsonValueObject>(PinObj));
		}
	}
	ResultObj->SetArrayField(TEXT("pins"), PinsArray);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// delete_node
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_DeleteNode::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title to delete."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_DeleteNode::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found in graph '%s'."), *NodeId, *GraphName));
		return false;
	}

	Node->BreakAllNodeLinks();
	Graph->RemoveNode(Node);
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("deleted_node_id"), NodeId);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// move_node
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_MoveNode::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> PosXProp = MakeShared<FJsonObject>();
	PosXProp->SetStringField(TEXT("type"), TEXT("number"));
	PosXProp->SetStringField(TEXT("description"), TEXT("New X position."));
	Props->SetObjectField(TEXT("pos_x"), PosXProp);

	TSharedPtr<FJsonObject> PosYProp = MakeShared<FJsonObject>();
	PosYProp->SetStringField(TEXT("type"), TEXT("number"));
	PosYProp->SetStringField(TEXT("description"), TEXT("New Y position."));
	Props->SetObjectField(TEXT("pos_y"), PosYProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("pos_x")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("pos_y")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_MoveNode::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	int32 PosX = InArgs->GetIntegerField(TEXT("pos_x"));
	int32 PosY = InArgs->GetIntegerField(TEXT("pos_y"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found."), *NodeId));
		return false;
	}

	Node->NodePosX = PosX;
	Node->NodePosY = PosY;
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("node_id"), NodeId);
	ResultObj->SetNumberField(TEXT("pos_x"), PosX);
	ResultObj->SetNumberField(TEXT("pos_y"), PosY);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// set_node_property
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_SetNodeProperty::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> PinProp = MakeShared<FJsonObject>();
	PinProp->SetStringField(TEXT("type"), TEXT("string"));
	PinProp->SetStringField(TEXT("description"), TEXT("Pin name to set literal value for (e.g. 'InString', 'Damage', 'Duration')."));
	Props->SetObjectField(TEXT("pin_name"), PinProp);

	TSharedPtr<FJsonObject> ValProp = MakeShared<FJsonObject>();
	ValProp->SetStringField(TEXT("type"), TEXT("string"));
	ValProp->SetStringField(TEXT("description"), TEXT("Literal value to assign (e.g. 'Hello World', '35.0', 'true')."));
	Props->SetObjectField(TEXT("value"), ValProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("pin_name")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("value")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_SetNodeProperty::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	FString PinName = InArgs->GetStringField(TEXT("pin_name"));
	FString Value = InArgs->GetStringField(TEXT("value"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found."), *NodeId));
		return false;
	}

	UEdGraphPin* Pin = FindNodePin(Node, PinName, EGPD_Input);
	if (!Pin)
	{
		Pin = FindNodePin(Node, PinName);
	}

	if (!Pin)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("PIN_NOT_FOUND"), FString::Printf(TEXT("Pin '%s' not found on node '%s'."), *PinName, *NodeId));
		return false;
	}

	Pin->DefaultValue = Value;
	if (Pin->GetSchema())
	{
		Pin->GetSchema()->TrySetDefaultValue(*Pin, Value);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("node_id"), NodeId);
	ResultObj->SetStringField(TEXT("pin_name"), PinName);
	ResultObj->SetStringField(TEXT("value"), Value);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// connect_pins
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_ConnectPins::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> SrcNodeProp = MakeShared<FJsonObject>();
	SrcNodeProp->SetStringField(TEXT("type"), TEXT("string"));
	SrcNodeProp->SetStringField(TEXT("description"), TEXT("Source node GUID or Title."));
	Props->SetObjectField(TEXT("source_node"), SrcNodeProp);

	TSharedPtr<FJsonObject> SrcPinProp = MakeShared<FJsonObject>();
	SrcPinProp->SetStringField(TEXT("type"), TEXT("string"));
	SrcPinProp->SetStringField(TEXT("description"), TEXT("Source pin name (e.g. 'then', 'ReturnValue', 'Damage')."));
	Props->SetObjectField(TEXT("source_pin"), SrcPinProp);

	TSharedPtr<FJsonObject> TgtNodeProp = MakeShared<FJsonObject>();
	TgtNodeProp->SetStringField(TEXT("type"), TEXT("string"));
	TgtNodeProp->SetStringField(TEXT("description"), TEXT("Target node GUID or Title."));
	Props->SetObjectField(TEXT("target_node"), TgtNodeProp);

	TSharedPtr<FJsonObject> TgtPinProp = MakeShared<FJsonObject>();
	TgtPinProp->SetStringField(TEXT("type"), TEXT("string"));
	TgtPinProp->SetStringField(TEXT("description"), TEXT("Target pin name (e.g. 'execute', 'Condition', 'InString')."));
	Props->SetObjectField(TEXT("target_pin"), TgtPinProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("source_node")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("source_pin")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("target_node")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("target_pin")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_ConnectPins::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString SrcNodeId = InArgs->GetStringField(TEXT("source_node"));
	FString SrcPinName = InArgs->GetStringField(TEXT("source_pin"));
	FString TgtNodeId = InArgs->GetStringField(TEXT("target_node"));
	FString TgtPinName = InArgs->GetStringField(TEXT("target_pin"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* SrcNode = FindGraphNode(Graph, SrcNodeId);
	if (!SrcNode)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("SOURCE_NODE_NOT_FOUND"), FString::Printf(TEXT("Source node '%s' not found."), *SrcNodeId));
		return false;
	}

	UEdGraphNode* TgtNode = FindGraphNode(Graph, TgtNodeId);
	if (!TgtNode)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("TARGET_NODE_NOT_FOUND"), FString::Printf(TEXT("Target node '%s' not found."), *TgtNodeId));
		return false;
	}

	UEdGraphPin* SrcPin = FindNodePin(SrcNode, SrcPinName, EGPD_Output);
	if (!SrcPin)
	{
		SrcPin = FindNodePin(SrcNode, SrcPinName);
	}

	if (!SrcPin)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("SOURCE_PIN_NOT_FOUND"), FString::Printf(TEXT("Source pin '%s' not found on node '%s'."), *SrcPinName, *SrcNodeId));
		return false;
	}

	UEdGraphPin* TgtPin = FindNodePin(TgtNode, TgtPinName, EGPD_Input);
	if (!TgtPin)
	{
		TgtPin = FindNodePin(TgtNode, TgtPinName);
	}

	if (!TgtPin)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("TARGET_PIN_NOT_FOUND"), FString::Printf(TEXT("Target pin '%s' not found on node '%s'."), *TgtPinName, *TgtNodeId));
		return false;
	}

	const UEdGraphSchema* Schema = Graph->GetSchema();
	if (!Schema)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("SCHEMA_ERROR"), TEXT("Graph schema is null."));
		return false;
	}

	bool bConnected = Schema->TryCreateConnection(SrcPin, TgtPin);
	if (!bConnected)
	{
		OutResult = FAIToolResult::Failure(
			GetName(),
			OutResult.ToolCallId,
			TEXT("CONNECTION_REJECTED"),
			FString::Printf(TEXT("Schema rejected connection between source pin '%s' (%s) on node '%s' and target pin '%s' (%s) on node '%s'."),
				*SrcPinName, *SrcPin->PinType.PinCategory.ToString(), *SrcNodeId,
				*TgtPinName, *TgtPin->PinType.PinCategory.ToString(), *TgtNodeId)
		);
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("source_node"), SrcNodeId);
	ResultObj->SetStringField(TEXT("source_pin"), SrcPinName);
	ResultObj->SetStringField(TEXT("target_node"), TgtNodeId);
	ResultObj->SetStringField(TEXT("target_pin"), TgtPinName);
	ResultObj->SetBoolField(TEXT("connected"), true);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// disconnect_pins
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_DisconnectPins::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NodeProp = MakeShared<FJsonObject>();
	NodeProp->SetStringField(TEXT("type"), TEXT("string"));
	NodeProp->SetStringField(TEXT("description"), TEXT("Node GUID or Title."));
	Props->SetObjectField(TEXT("node_id"), NodeProp);

	TSharedPtr<FJsonObject> PinProp = MakeShared<FJsonObject>();
	PinProp->SetStringField(TEXT("type"), TEXT("string"));
	PinProp->SetStringField(TEXT("description"), TEXT("Pin name to disconnect."));
	Props->SetObjectField(TEXT("pin_name"), PinProp);

	TSharedPtr<FJsonObject> GraphProp = MakeShared<FJsonObject>();
	GraphProp->SetStringField(TEXT("type"), TEXT("string"));
	GraphProp->SetStringField(TEXT("description"), TEXT("Optional graph name (default: EventGraph)."));
	Props->SetObjectField(TEXT("graph_name"), GraphProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("node_id")));
	Required.Add(MakeShared<FJsonValueString>(TEXT("pin_name")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_DisconnectPins::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));
	FString NodeId = InArgs->GetStringField(TEXT("node_id"));
	FString PinName = InArgs->GetStringField(TEXT("pin_name"));
	FString GraphName = InArgs->HasField(TEXT("graph_name")) ? InArgs->GetStringField(TEXT("graph_name")) : TEXT("EventGraph");

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	UEdGraph* Graph = FindBlueprintGraph(BP, GraphName);
	if (!Graph)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("GRAPH_NOT_FOUND"), FString::Printf(TEXT("Graph '%s' not found."), *GraphName));
		return false;
	}

	UEdGraphNode* Node = FindGraphNode(Graph, NodeId);
	if (!Node)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("NODE_NOT_FOUND"), FString::Printf(TEXT("Node '%s' not found."), *NodeId));
		return false;
	}

	UEdGraphPin* Pin = FindNodePin(Node, PinName);
	if (!Pin)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("PIN_NOT_FOUND"), FString::Printf(TEXT("Pin '%s' not found on node '%s'."), *PinName, *NodeId));
		return false;
	}

	Pin->BreakAllPinLinks();
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("node_id"), NodeId);
	ResultObj->SetStringField(TEXT("pin_name"), PinName);
	ResultObj->SetBoolField(TEXT("disconnected"), true);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// compile_blueprint
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_CompileBlueprint::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path to compile."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_CompileBlueprint::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	FCompilerResultsLog ResultsLog;
	FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::None, &ResultsLog);

	bool bCompiledCleanly = (BP->Status == BS_UpToDate);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetBoolField(TEXT("compile_success"), bCompiledCleanly);
	ResultObj->SetNumberField(TEXT("error_count"), ResultsLog.NumErrors);
	ResultObj->SetNumberField(TEXT("warning_count"), ResultsLog.NumWarnings);

	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const TSharedRef<FTokenizedMessage>& MsgRef : ResultsLog.Messages)
	{
		TSharedPtr<FJsonObject> MsgObj = MakeShared<FJsonObject>();
		MsgObj->SetStringField(TEXT("message"), MsgRef->ToText().ToString());
		MsgObj->SetStringField(TEXT("severity"), MsgRef->GetSeverity() == EMessageSeverity::Error ? TEXT("Error") : (MsgRef->GetSeverity() == EMessageSeverity::Warning ? TEXT("Warning") : TEXT("Info")));
		MessagesArray.Add(MakeShared<FJsonValueObject>(MsgObj));
	}
	ResultObj->SetArrayField(TEXT("compiler_messages"), MessagesArray);

	if (!bCompiledCleanly)
	{
		OutResult = FAIToolResult::Failure(
			GetName(),
			OutResult.ToolCallId,
			TEXT("COMPILE_ERROR"),
			FString::Printf(TEXT("Blueprint '%s' failed compilation with %d error(s). Please review compiler_messages and repair nodes/connections."), *BPPath, ResultsLog.NumErrors)
		);
		return false;
	}

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}

// -----------------------------------------------------------------------------
// get_compile_results
// -----------------------------------------------------------------------------
TSharedPtr<FJsonObject> FUnrealAITool_GetCompileResults::GetParametersSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> PathProp = MakeShared<FJsonObject>();
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Blueprint asset path."));
	Props->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Props);

	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("blueprint_path")));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}

bool FUnrealAITool_GetCompileResults::Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult)
{
	FString BPPath = InArgs->GetStringField(TEXT("blueprint_path"));

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BPPath);
	if (!BP)
	{
		OutResult = FAIToolResult::Failure(GetName(), OutResult.ToolCallId, TEXT("BLUEPRINT_NOT_FOUND"), FString::Printf(TEXT("Could not load Blueprint at '%s'."), *BPPath));
		return false;
	}

	FString StatusStr = TEXT("Unknown");
	switch (BP->Status)
	{
	case BS_UpToDate: StatusStr = TEXT("UpToDate"); break;
	case BS_Dirty: StatusStr = TEXT("Dirty"); break;
	case BS_Error: StatusStr = TEXT("Error"); break;
	case BS_UpToDateWithWarnings: StatusStr = TEXT("UpToDateWithWarnings"); break;
	default: break;
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_path"), BPPath);
	ResultObj->SetStringField(TEXT("status"), StatusStr);
	ResultObj->SetBoolField(TEXT("is_up_to_date"), BP->Status == BS_UpToDate || BP->Status == BS_UpToDateWithWarnings);

	OutResult = FAIToolResult::Success(GetName(), OutResult.ToolCallId, ResultObj);
	return true;
}
