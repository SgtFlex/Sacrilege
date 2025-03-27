// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HaloGameState.h"
#include "FirefightGameState.generated.h"
class ACharacterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChanged, int, CurrentWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetChanged, int, CurrentSet);
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API AFirefightGameState : public AHaloGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentWave();

	UFUNCTION(NetMulticast, Reliable)
	void SetCurrentWave(int NewWave);

	UFUNCTION(BlueprintImplementableEvent)
	void WaveStarted();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentSet();

	UFUNCTION(NetMulticast, Reliable)
	void SetCurrentSet(int NewSet);
	

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentEnemyCount();

	UFUNCTION(BlueprintImplementableEvent)
	void SetStarted();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetFinished();

	UFUNCTION(BlueprintImplementableEvent)
	void MatchStarted();

	
public:
	UPROPERTY(BlueprintAssignable)
	FOnSetChanged OnSetChanged;

	UPROPERTY(BlueprintAssignable)
	FOnWaveChanged OnWaveChanged;

protected:	
	UPROPERTY(Replicated)
	int curWave = 0;

	UPROPERTY(Replicated)
	int curSet = 0;

	UPROPERTY(BlueprintReadWrite, Replicated)
	int CurrentEnemyCount = 0;
};
