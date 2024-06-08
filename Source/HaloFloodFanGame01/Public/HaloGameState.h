// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HaloGameState.generated.h"

class ACharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChanged, int, CurrentWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetChanged, int, CurrentSet);


UCLASS()
class HALOFLOODFANGAME01_API AHaloGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
	
	UFUNCTION(BlueprintGetter)
	int GetCurrentWave();

	UFUNCTION(BlueprintSetter)
	int SetCurrentWave(int NewWave);

	UFUNCTION(BlueprintGetter)
	int GetCurrentSet();

	UFUNCTION(BlueprintSetter)
	int SetCurrentSet(int NewSet);

	UFUNCTION(BlueprintGetter)
	int GetCurrentEnemyCount();

	
	
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
