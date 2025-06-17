// Fill out your copyright notice in the Description page of Project Settings.


#include "FirefightGameState.h"
#include "NotificationSubsystem.h"
#include "Net/UnrealNetwork.h"

void AFirefightGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirefightGameState, curWave);
	DOREPLIFETIME(AFirefightGameState, curSet);
	DOREPLIFETIME(AFirefightGameState, CurrentEnemyCount);
}

int AFirefightGameState::GetCurrentWave()
{
	return curWave;
}

void AFirefightGameState::SetCurrentWave_Implementation(int NewWave)
{
	if (HasAuthority())
	{
		curWave = NewWave;
	}
	WaveStarted();
	OnWaveChanged.Broadcast(curWave);
}


int AFirefightGameState::GetCurrentSet()
{
	return curSet;
}

void AFirefightGameState::SetPlayerLives_Implementation(int NewPlayerLives)
{
	if (HasAuthority())
	{
		CurPlayerLives = NewPlayerLives;
	}
}

int AFirefightGameState::GetCurrentPlayerLives()
{
	return CurPlayerLives;
}

void AFirefightGameState::SetCurrentSet_Implementation(int NewSet)
{
	SetStarted();
	if (HasAuthority())
	{
		curSet = NewSet;
	}
	UE_LOG(LogTemp, Warning, TEXT("Set Start: %d"), curSet);
	OnSetChanged.Broadcast(curSet);
}

void AFirefightGameState::SetCurrentEnemyCount_Implementation(int EnemyCount)
{
	CurrentEnemyCount = EnemyCount;
}

int AFirefightGameState::GetCurrentEnemyCount()
{
	return CurrentEnemyCount;
}
