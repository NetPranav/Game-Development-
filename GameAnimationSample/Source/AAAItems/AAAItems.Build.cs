using UnrealBuildTool;

public class AAAItems : ModuleRules
{
	public AAAItems(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"GameplayTags",
			"GameplayAbilities",
			"GameplayTasks",
			"NetCore"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore" });
	}
}
