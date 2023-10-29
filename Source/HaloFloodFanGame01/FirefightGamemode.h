// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "FirefightGamemode.generated.h"

class ACharacterBase;
class AAISpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStart, int, CurrentSet, int, CurrentWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreUpdated, APlayerController*, PlayerController, int, NewScore, int, NewResource);

USTRUCT(BlueprintType)
struct FSquadStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ACharacterBase>, int> SquadUnits;
	
};

UCLASS(minimalapi)
class AFirefightGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFirefightGameMode();

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyKilled(AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	int GetCurrentWave();
	void StartSet();
	void FinishSet();
	void StartWave();
	void FinishWave();
	int GetCurrentSet();
	TArray<FSquadStruct> CalculateWave();
	void SpawnWave(TArray<FSquadStruct> WaveToSpawn);
	void OnSpawnerAvailable(AAISpawner* Spawner);
	void GameFinished();

	UFUNCTION(BlueprintGetter)
	int GetPlayerResource(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	int SetPlayerResource(APlayerController* PlayerController, int NewPlayerResource);

	UFUNCTION()
	void TestFunc(ACharacterBase* Character);

	UFUNCTION(BlueprintGetter)
	int GetPlayerScore(APlayerController* PlayerController);

	UFUNCTION()
	UAudioComponent* GetSoundtrackComponent();

	FTimerHandle SetFinishDelayTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable)
	FOnScoreUpdated OnScoreUpdated;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FOnWaveStart OnWaveStart;

	UPROPERTY(BlueprintReadOnly)
	int curWave = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxWave = 5;
	UPROPERTY(BlueprintReadOnly)
	int curSet = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxSet = 3;

	UPROPERTY(BlueprintReadWrite)
	int PlayerResource = 0;

	UPROPERTY(BlueprintReadWrite)
	int PlayerScore = 0;
private:
	

	int MaxWavePool = 5;
	int CurWavePool = 5;
	int MaxSquadCost = 1;
	
	UPROPERTY(EditDefaultsOnly)
	bool bEnableMusic = true;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSquadStruct> SquadPool;

	TArray<FSquadStruct> SquadsToSpawn;

	TArray<FSquadStruct> SquadsAtWaveStart;

	UPROPERTY(BlueprintReadWrite)
	int CurrentEnemyCount = 0;

	UPROPERTY()
	int WaveEnemyCount;
	
	TArray<AAISpawner*> Spawners;

	TArray<AAISpawner*> AvailableSpawners;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<USoundBase*> Soundtracks;

	UPROPERTY()
	UAudioComponent* SoundtrackComponent;
};



