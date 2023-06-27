// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01GameMode.generated.h"

class ABaseCharacter;
class AHaloSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStart, int, CurrentSet, int, CurrentWave);

USTRUCT(BlueprintType)
struct FSquadStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ABaseCharacter>, int> SquadUnits;
	
};

UCLASS(minimalapi)
class AHaloFloodFanGame01GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AHaloFloodFanGame01GameMode();

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyKilled();
	int GetCurrentWave();
	void StartSet();
	void FinishSet();
	void StartWave();
	void FinishWave();
	int GetCurrentSet();
	TArray<FSquadStruct> CalculateWave();
	void SpawnWave(TArray<FSquadStruct> WaveToSpawn);
	void OnSpawnerAvailable(AHaloSpawner* Spawner);
	void GameFinished();

	UFUNCTION()
	UAudioComponent* GetSoundtrackComponent();

	FTimerHandle SetFinishDelayTimer;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FOnWaveStart OnWaveStart;

	
private:
	int curWave = 0;
	int maxWave = 3;
	int curSet = 0;
	int maxSet = 3;

	int MaxWavePool = 5;
	int CurWavePool = 5;
	int MaxSquadCost = 1;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSquadStruct> SquadPool;

	TArray<FSquadStruct> SquadsToSpawn;

	TArray<FSquadStruct> SquadsAtWaveStart;

	UPROPERTY(BlueprintReadWrite)
	int CurrentEnemyCount = 0;

	UPROPERTY()
	int WaveEnemyCount;
	
	TArray<AHaloSpawner*> Spawners;

	TArray<AHaloSpawner*> AvailableSpawners;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<USoundBase*> Soundtracks;

	UPROPERTY()
	UAudioComponent* SoundtrackComponent;
};



