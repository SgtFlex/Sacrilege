// Fill out your copyright notice in the Description page of Project Settings.


#include "Gamemodes/HaloPlayerState.h"

#include "Net/UnrealNetwork.h"

void AHaloPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHaloPlayerState, PlayerScore);
	DOREPLIFETIME(AHaloPlayerState, PlayerResource);
}

void AHaloPlayerState::AddPlayerScore(int AddScore)
{
	SetPlayerScore(PlayerScore+AddScore);
}

void AHaloPlayerState::SubtractPlayerScore(int SubtractScore)
{
	SetPlayerScore(PlayerScore-SubtractScore);
}

void AHaloPlayerState::SetPlayerScore_Implementation(int NewScore)
{
	const int ScoreGained = NewScore - PlayerScore;
	PlayerScore = NewScore;
	OnScoreUpdated.Broadcast(this, GetPlayerScore(), ScoreGained);
}

int AHaloPlayerState::GetPlayerScore()
{
	return PlayerScore;
}

void AHaloPlayerState::AddPlayerResource(int AddResource)
{
	SetPlayerResource(PlayerResource+AddResource);
}

void AHaloPlayerState::SubtractPlayerResource(int SubtractResource)
{
	SetPlayerResource(PlayerResource-SubtractResource);
}

void AHaloPlayerState::SetPlayerResource_Implementation(int NewResource)
{
	const int ResourceGained = NewResource - PlayerResource;
	PlayerResource = NewResource;
	OnResourceUpdated.Broadcast(this, GetPlayerResource(), ResourceGained);
}


int AHaloPlayerState::GetPlayerResource()
{
	return PlayerResource;
}

bool AHaloPlayerState::HasPlayerResource(int Amount)
{
	return PlayerResource >= Amount;
}

void AHaloPlayerState::SetPlayerID(const FString& NewName)
{
	SetPlayerName(NewName);
}
