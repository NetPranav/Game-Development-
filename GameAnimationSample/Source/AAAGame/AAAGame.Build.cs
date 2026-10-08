using UnrealBuildTool;

public class AAAGame : ModuleRules
{
	public AAAGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "AAAAbility", "AAACombat", "AIModule" });
	}
}
