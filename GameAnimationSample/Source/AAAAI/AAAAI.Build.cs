using UnrealBuildTool;

public class AAAAI : ModuleRules
{
	public AAAAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "GameplayAbilities", "GameplayTags", "GameplayTasks" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "AAACharacter", "AAAAbility" });
	}
}
