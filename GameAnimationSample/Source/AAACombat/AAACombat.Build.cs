using UnrealBuildTool;

public class AAACombat : ModuleRules
{
	public AAACombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayAbilities", "GameplayTags", "GameplayTasks", "DeveloperSettings", "AIModule", "GameplayCameras", "EngineCameras" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore" });
	}
}
