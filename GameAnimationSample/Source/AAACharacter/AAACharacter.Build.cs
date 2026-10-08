using UnrealBuildTool;

public class AAACharacter : ModuleRules
{
	public AAACharacter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "AIModule" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "AAAGame", "AAAAbility", "AAAInventory", "AAAInteraction", "AAACombat" });
	}
}
