// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RRSettings.generated.h"

class ARREnemy;

/**
 * Settings for the RRGame module.
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="RR Game Settings"))
class RRGAME_API URRSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URRSettings();

	static const URRSettings* Get() { return GetDefault<URRSettings>(); }

	/** Default enemy class to spawn for tests */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Enemy")
	TSoftClassPtr<ARREnemy> DefaultEnemyClass;

	UPROPERTY(Config, EditAnywhere, Category = "RR|Boss")
	TSoftClassPtr<class ARRBoss> BossClass;

	UPROPERTY(Config, EditAnywhere, Category = "RR|Boss")
	bool bBossMode = true;

	/** Maximum number of enemies that can attack the player simultaneously */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Enemy")
	int32 MaxSimultaneousAttackers;

	/** Display actor (retargeted body) shown instead of the mannequin for the player. */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Visual")
	TSoftClassPtr<AActor> PlayerVisualClass;

	/** URL for the local Sui sidecar API */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Sui")
	FString SuiSidecarUrl = TEXT("http://127.0.0.1:3001");

	/** Spawn ambient NPCs (Game Animation Sample characters, one per look) around the arena when the match starts. */
	UPROPERTY(Config, EditAnywhere, Category = "RR|NPC")
	bool bSpawnNpcs = true;

	/** Pawn used for NPCs: the sample's CharacterMovement sandbox character (motion matching locomotion). */
	UPROPERTY(Config, EditAnywhere, Category = "RR|NPC")
	TSoftClassPtr<APawn> NpcPawnClass;

	/** One NPC is spawned per entry (retargeted display actors: UE4 mannequin, Quinn, Echo, Twinblast, MetaHuman, robot...). */
	UPROPERTY(Config, EditAnywhere, Category = "RR|NPC")
	TArray<TSoftClassPtr<AActor>> NpcVisualClasses;

	/** Distance of the NPC ring from the player start (cm). */
	UPROPERTY(Config, EditAnywhere, Category = "RR|NPC")
	float NpcRingRadius = 1400.0f;

	/** Cinematic post process (lens + focus on the player) spawned at match start. */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Visual")
	bool bCinematicLook = true;

	/** Meshes for swords A, B, and Fab Longsword */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Visual")
	TArray<TSoftObjectPtr<UStaticMesh>> SwordMeshes;

	/** Mesh for player shield */
	UPROPERTY(Config, EditAnywhere, Category = "RR|Visual")
	TSoftObjectPtr<UStaticMesh> ShieldMesh;
};
