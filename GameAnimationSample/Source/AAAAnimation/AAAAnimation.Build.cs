using UnrealBuildTool;

public class AAAAnimation : ModuleRules
{
	public AAAAnimation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "AAACharacter", "AAACombat", "GameplayAbilities", "GameplayTags", "GameplayTasks" });
	}
}
