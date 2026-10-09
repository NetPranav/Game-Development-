// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRSettings.h"
#include "RREnemy.h"

URRSettings::URRSettings()
{
	MaxSimultaneousAttackers = 2;
	PlayerVisualClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")));
	BossClass = TSoftClassPtr<class ARRBoss>(FSoftObjectPath(TEXT("/Game/RR/Blueprints/BP_RRBoss.BP_RRBoss_C")));
	bBossMode = true;
	bSpawnNpcs = true;

	NpcPawnClass = TSoftClassPtr<APawn>(FSoftObjectPath(TEXT("/Game/Blueprints/SandboxCharacter_CMC.SandboxCharacter_CMC_C")));
	for (const TCHAR* Path : {
		TEXT("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin.BP_UE4_Mannequin_C"),
		TEXT("/Game/Blueprints/RetargetedCharacters/BP_Quinn.BP_Quinn_C"),
		TEXT("/Game/Blueprints/RetargetedCharacters/BP_Echo.BP_Echo_C"),
		TEXT("/Game/Blueprints/RetargetedCharacters/BP_Twinblast.BP_Twinblast_C"),
		TEXT("/Game/MetaHumans/Kellan/BP_Kellan.BP_Kellan_C"),
		TEXT("/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C") })
	{
		NpcVisualClasses.Add(TSoftClassPtr<AActor>(FSoftObjectPath(Path)));
	}

	SwordMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A.SM_Sword_A"))));
	SwordMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B.SM_Sword_B"))));
	SwordMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/RR/Weapons/Fab/long_sword/StaticMeshes/SM_LongSword.SM_LongSword"))));
	ShieldMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/RR/Weapons/Fab/viking_shield/StaticMeshes/SM_VikingShield.SM_VikingShield")));
}
