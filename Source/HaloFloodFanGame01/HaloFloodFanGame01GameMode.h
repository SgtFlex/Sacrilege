// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01GameMode.generated.h"

class ABaseCharacter;
class AHaloSpawner;

USTRUCT(BlueprintType)
struct FWaveStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ABaseCharacter>, int> Squad;
};

UCLASS(minimalapi)
class AHaloFloodFanGame01GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHaloFloodFanGame01GameMode();

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyKilled();
	int GetCurrentWave();
	void WaveFinished();
	int GetCurrentSet();
	void SpawnWave(TArray<FWaveStruct> WaveToSpawn);
	void GameFinished();

private:
	int curWave = 0;
	int maxWave = 3;
	int curSet = 0;
	int maxSet = 3;

	int CurrentEnemyCount = 0;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveStruct> Wave;
	
	TArray<AHaloSpawner*> Spawners;
};



