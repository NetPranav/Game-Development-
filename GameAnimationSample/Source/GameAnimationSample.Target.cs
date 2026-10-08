// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class GameAnimationSampleTarget : TargetRules
{
	public GameAnimationSampleTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		ExtraModuleNames.AddRange( new string[] { 
			"GameAnimationSample", 
			"AAACore", 
			"AAAGame", 
			"AAACharacter", 
			"AAAAnimation", 
			"AAACombat", 
			"AAAAbility", 
			"AAAInventory", 
			"AAAItems", 
			"AAAInteraction", 
			"AAAAI", 
			"AAANetwork", 
			"AAAWorld", 
			"AAAUI",
			"RRGame"
		} );
	}
}
