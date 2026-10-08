// Copyright (c) 2026 Antigravity. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Registry/UnrealAITool.h"

class FUnrealAITool_GetGraphNodes : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_graph_nodes"); }
	virtual FString GetDescription() const override { return TEXT("List all nodes in a specified graph of a Blueprint (IDs, Titles, Classes, Graph coordinates)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_GetNodeInfo : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_node_info"); }
	virtual FString GetDescription() const override { return TEXT("Get comprehensive information for a specific node (GUID, Class, Comment, ErrorState, Input/Output Pins)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_GetNodePins : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_node_pins"); }
	virtual FString GetDescription() const override { return TEXT("Get detailed pin information for a node (PinName, Direction, PinType, DefaultValue, LinkedNodes, LinkedPins)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_FindNode : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("find_node"); }
	virtual FString GetDescription() const override { return TEXT("Search for nodes in a Blueprint graph matching a name, function, or class query."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_FindPin : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("find_pin"); }
	virtual FString GetDescription() const override { return TEXT("Find a specific pin on a node by pin name or direction."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_CreateNode : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("create_node"); }
	virtual FString GetDescription() const override { return TEXT("Create a new Blueprint graph node (e.g. CallFunction, CustomEvent, Event, VariableGet, VariableSet, Branch/IfThenElse, Sequence, DynamicCast, Math, etc.)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_DeleteNode : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("delete_node"); }
	virtual FString GetDescription() const override { return TEXT("Delete a node from a Blueprint graph and break all its connections safely."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_MoveNode : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("move_node"); }
	virtual FString GetDescription() const override { return TEXT("Move a node to a new graph coordinate position (X, Y)."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_SetNodeProperty : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("set_node_property"); }
	virtual FString GetDescription() const override { return TEXT("Set a literal default value for an input pin on a node (e.g. Damage = 35.0, String, Vector, Boolean, etc.) or node comment."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_ConnectPins : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("connect_pins"); }
	virtual FString GetDescription() const override { return TEXT("Connect two Blueprint node pins together using schema compatibility validation."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_DisconnectPins : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("disconnect_pins"); }
	virtual FString GetDescription() const override { return TEXT("Break connections between two pins, or break all links on a specific pin."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_CompileBlueprint : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("compile_blueprint"); }
	virtual FString GetDescription() const override { return TEXT("Compile a Blueprint and capture compilation results, error messages, and warnings."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::SafeWrite; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};

class FUnrealAITool_GetCompileResults : public IUnrealAITool
{
public:
	virtual FString GetName() const override { return TEXT("get_compile_results"); }
	virtual FString GetDescription() const override { return TEXT("Get latest compilation status, error messages, and warning diagnostics for a Blueprint."); }
	virtual TSharedPtr<FJsonObject> GetParametersSchema() const override;
	virtual EUnrealAIToolSafety GetSafetyLevel() const override { return EUnrealAIToolSafety::ReadOnly; }
	virtual bool Execute(const TSharedPtr<FJsonObject>& InArgs, FAIToolResult& OutResult) override;
};
