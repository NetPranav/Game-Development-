using UnrealBuildTool;

public class AAAAbility : ModuleRules
{
	public AAAAbility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "GameplayAbilities", "GameplayTags", "GameplayTasks", "EnhancedInput", "AAACombat", "Mover", "AIModule" });
	}
}
