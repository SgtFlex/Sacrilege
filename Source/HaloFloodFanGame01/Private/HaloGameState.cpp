// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloGameState.h"

#include "Net/UnrealNetwork.h"

void AHaloGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHaloGameState, curWave);
	DOREPLIFETIME(AHaloGameState, curSet);
}

int AHaloGameState::GetCurrentWave()
{
	return curWave;
}

int AHaloGameState::SetCurrentWave(int NewWave)
{
	
	curWave = NewWave;
	OnWaveChanged.Broadcast(curWave);
	return curWave;
}


int AHaloGameState::GetCurrentSet()
{
	return curSet;
}

int AHaloGameState::SetCurrentSet(int NewSet)
{
	curSet = NewSet;
	OnSetChanged.Broadcast(curSet);
	return curSet;
}

int AHaloGameState::GetCurrentEnemyCount()
{
	return CurrentEnemyCount;
}
