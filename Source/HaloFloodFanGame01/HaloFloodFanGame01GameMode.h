// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01GameMode.generated.h"

class ABaseCharacter;
class AHaloSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStart, int, CurrentWave, int, CurrentSet);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FOnWaveStart OnWaveStart;

private:
	int curWave = 0;
	int maxWave = 3;
	int curSet = 0;
	int maxSet = 3;

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveStruct> Wave;

	UPROPERTY(BlueprintReadWrite)
	int CurrentEnemyCount = 0;
	
	TArray<AHaloSpawner*> Spawners;
};



