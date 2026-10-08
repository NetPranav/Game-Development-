// Copyright (c) 2026 Antigravity. All Rights Reserved.

using UnrealBuildTool;

public class UnrealAIAgentCore : ModuleRules
{
	public UnrealAIAgentCore(ReadOnlyTargetRules Target) : base(Target)
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
				"HTTP",
				"Json",
				"JsonUtilities",
				"DeveloperSettings"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Private dependencies
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
