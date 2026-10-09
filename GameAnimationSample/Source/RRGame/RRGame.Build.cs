using UnrealBuildTool;

public class RRGame : ModuleRules
{
	public RRGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HTTP", "Json", "JsonUtilities", "UMG", "Slate", "SlateCore", "GameplayTags", "GameplayAbilities", "AIModule", "NavigationSystem", "DeveloperSettings", "ApplicationCore", "RenderCore", "MediaAssets" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "AAACore", "AAACharacter", "AAACombat", "AAAGame", "AAAAI", "AAAItems", "AAAAbility" });
	}
}
