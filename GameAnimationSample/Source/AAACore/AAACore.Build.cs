using UnrealBuildTool;

public class AAACore : ModuleRules
{
	public AAACore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "GameplayTags" });
		
		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
