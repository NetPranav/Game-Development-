// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "RRSettings.h"
#include "RREnemy.h"

URRSettings::URRSettings()
{
	MaxSimultaneousAttackers = 2;
	PlayerVisualClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/RR/Blueprints/BP_RobotScout.BP_RobotScout_C")));
	BossClass = TSoftClassPtr<class ARRBoss>(FSoftObjectPath(TEXT("/Game/RR/Blueprints/BP_RRBoss.BP_RRBoss_C")));
	bBossMode = true;

	SwordMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A.SM_Sword_A"))));
	SwordMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B.SM_Sword_B"))));
}
