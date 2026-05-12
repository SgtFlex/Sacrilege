// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gamemodes/HaloGameMode.h"
#include "FirefightGamemode.generated.h"

class AFirefightGameState;
class AHaloGameState;
class AVehicleBase;
class AGunBase;
class ACharacterBase;
class AAISpawner;
class APlayerCharacter;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerCharDied, ACharacterBase*, PlayerCharacter, APlayerControllerBase*, PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnemyKilled, ACharacterBase*, Character, AController*, EventInstigator, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSetEnd);


USTRUCT(BlueprintType)
struct FSquadStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ACharacterBase>, int> SquadUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AVehicleBase>, int> SquadVehicles;
	
};

UCLASS(minimalapi)
class AFirefightGameMode : public AHaloGameMode
{
	GENERATED_BODY()

public:
	AFirefightGameMode();
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void EnemyKilled(ACharacterBase* Character = nullptr, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	virtual void StartMatch() override;

	UFUNCTION()
	void StartSet();

	UFUNCTION()
	void FinishSet();

	UFUNCTION(BlueprintImplementableEvent)
	void SetIntermission();

	UFUNCTION()
	void StartWave();

	UFUNCTION()
	void FinishWave();

	virtual bool ReadyToStartMatch_Implementation() override;

	UFUNCTION()
	TArray<FSquadStruct> CalculateWave();

	UFUNCTION()
	void SpawnWave(TArray<FSquadStruct> WaveToSpawn);

	UFUNCTION()
	void CharacterDestroyed(AActor* DestroyedActor);
	UFUNCTION(BlueprintCallable)
	void ManageCharacter(ACharacterBase* Character);

	UFUNCTION()
	void OnSpawnerAvailable(AAISpawner* Spawner);

	UFUNCTION()
	void GameFinished();

	virtual void OnPostLogin(AController* NewPlayer) override;

	virtual void RestartPlayer(AController* NewPlayer) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void RestartPlayerBP(AController* NewPlayer);

	UFUNCTION(BlueprintNativeEvent)
	void PlayerDied(ACharacterBase* PlayerCharacter, APlayerControllerBase* PlayerController);

	UFUNCTION(BlueprintImplementableEvent)
	APawn* CreateSpectator(ACharacterBase* Character, APlayerControllerBase* PlayerController, FTimerHandle RespawnTimerHandle);

	UFUNCTION(BlueprintCallable)
	bool SpawnPlayer(APlayerControllerBase* PlayerController);

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
	UPROPERTY(BlueprintAssignable)
	FOnSetEnd OnSetEnd;
	
	UPROPERTY()
	FTimerHandle SetFinishDelayTimer;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerCharDied OnPlayerCharDied;

	UPROPERTY(BlueprintAssignable)
	FOnEnemyKilled OnEnemyKilled;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxWave = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int maxSet = 3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RespawnTime = 3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<AVehicleBase> RespawnVehicle;

	//UPROPERTY(BlueprintReadOnly)
	//FTimerHandle RespawnTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int PlayerLives = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AGunBase>> StartingPrimaryWeaponChoices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AGunBase>> StartingSecondaryWeaponChoices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<ACharacterBase>> StartingCharacterChoices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSquadStruct> SquadPool;

	UPROPERTY()
	TArray<FSquadStruct> SquadsToSpawn;

	UPROPERTY()
	TArray<FSquadStruct> SquadsAtWaveStart;

	UPROPERTY(BlueprintReadWrite)
	int CurrentEnemyCount = 0;

	UPROPERTY()
	int WaveEnemyCount;

	UPROPERTY()
	TArray<AAISpawner*> Spawners;

	UPROPERTY()
	TArray<AAISpawner*> AvailableSpawners;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadoutScreenClass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<ACharacterBase*> FirefightEnemies;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<TSubclassOf<ACharacterBase>, int> ScoreMap;
	
private:
	int MaxWavePool = 5;
	int CurWavePool = 5;
	int MaxSquadCost = 1;

protected:
	UPROPERTY()
	AFirefightGameState* FirefightGameState;
};



