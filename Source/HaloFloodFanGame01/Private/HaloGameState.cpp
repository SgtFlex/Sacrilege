// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloGameState.h"

#include "NotificationSubsystem.h"
#include "Net/UnrealNetwork.h"

void AHaloGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHaloGameState, curWave);
	DOREPLIFETIME(AHaloGameState, curSet);
	DOREPLIFETIME(AHaloGameState, CurrentEnemyCount);
}

int AHaloGameState::GetCurrentWave()
{
	return curWave;
}

void AHaloGameState::SetCurrentWave_Implementation(int NewWave)
{
	if (HasAuthority())
	{
		curWave = NewWave;
	}
	GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Reinforcements");
	OnWaveChanged.Broadcast(curWave);
}


int AHaloGameState::GetCurrentSet()
{
	return curSet;
}

void AHaloGameState::SetCurrentSet_Implementation(int NewSet)
{
	if (HasAuthority())
	{
		curSet = NewSet;
	}
	GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Set start");
	OnSetChanged.Broadcast(curSet);
}

int AHaloGameState::GetCurrentEnemyCount()
{
	return CurrentEnemyCount;
}
