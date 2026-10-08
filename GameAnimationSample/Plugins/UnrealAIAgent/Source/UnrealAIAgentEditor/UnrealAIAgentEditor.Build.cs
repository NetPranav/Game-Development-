// Copyright (c) 2026 Antigravity. All Rights Reserved.

using UnrealBuildTool;

public class UnrealAIAgentEditor : ModuleRules
{
	public UnrealAIAgentEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// Public includes
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// Private includes
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealAIAgentCore",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorStyle",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"WorkspaceMenuStructure",
				"PropertyEditor",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser",
				"ContentBrowserData",
				"Kismet",
				"KismetCompiler",
				"BlueprintGraph",
				"GraphEditor",
				"Json",
				"JsonUtilities",
				"HTTP",
				"ApplicationCore"
			}

		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"DeveloperSettings"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
