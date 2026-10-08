using UnrealBuildTool;

public class AAAInventory : ModuleRules
{
	public AAAInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"GameplayTags",
			"NetCore",
			"AAAItems"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore" });
	}
}
