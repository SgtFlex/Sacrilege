// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HaloGameMode.h"
#include "FirefightGamemode.generated.h"

class AHaloGameState;
class AVehicleBase;
class AGunBase;
class ACharacterBase;
class AAISpawner;
class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStart, int, CurrentSet, int, CurrentWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerCharDied, ACharacterBase*, PlayerCharacter, APlayerControllerBase*, PlayerController);

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
class AFirefightGameMode : public AHaloGameMode
{
	GENERATED_BODY()

public:
	AFirefightGameMode();


	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyKilled(ACharacterBase* Character = nullptr, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	void StartMatch() override;
	void StartSet();
	void FinishSet();
	void StartWave();
	void FinishWave();
	TArray<FSquadStruct> CalculateWave();
	void SpawnWave(TArray<FSquadStruct> WaveToSpawn);

	UFUNCTION(BlueprintCallable)
	void ManageCharacter(ACharacterBase* Character);
	
	void OnSpawnerAvailable(AAISpawner* Spawner);
	void GameFinished();

	virtual void RestartPlayer(AController* NewPlayer) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void RestartPlayerBP(AController* NewPlayer);

	UFUNCTION(BlueprintNativeEvent)
	void PlayerDied(ACharacterBase* PlayerCharacter, APlayerControllerBase* PlayerController);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateSpectator(ACharacterBase* Character, APlayerControllerBase* PlayerController, float TimeToSpawn);

	UFUNCTION(BlueprintCallable)
	bool FinishSpawning(APlayerControllerBase* PlayerController, uint8 Team = 0, TSubclassOf<AGunBase> PrimaryWeaponClass = nullptr, TSubclassOf<AGunBase> SecondaryWeaponClass = nullptr, TSubclassOf<ACharacterBase> CharacterClass = nullptr);

	UFUNCTION(BlueprintCallable)
	void RespawnPlayer(APlayerControllerBase* PlayerController);

	UFUNCTION()
	void StartRespawnProcess(APlayerControllerBase* PC, ACharacterBase* PreviousCharacter = nullptr);
	
	UFUNCTION()
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	void EndGame();

	virtual void HandleMatchHasStarted() override;
	
	UFUNCTION(BlueprintNativeEvent)
	AVehicleBase* SpawnRespawnVehicle(APlayerController* PlayerController);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
public:
	FTimerHandle SetFinishDelayTimer;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerCharDied OnPlayerCharDied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FOnWaveStart OnWaveStart;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxWave = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxSet = 3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RespawnTime = 3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<AVehicleBase> RespawnVehicle;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle RespawnTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int PlayerLives = 5;

	int CurPlayerLives;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AGunBase>> StartingPrimaryWeaponChoices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AGunBase>> StartingSecondaryWeaponChoices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<ACharacterBase>> StartingCharacterChoices;

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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadoutScreenClass;
	
private:
	int MaxWavePool = 5;
	int CurWavePool = 5;
	int MaxSquadCost = 1;

protected:
	UPROPERTY()
	AHaloGameState* HaloGameState;
};



