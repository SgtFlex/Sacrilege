// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01GameMode.h"

#include "HaloSpawner.h"
#include "Core/BaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet\GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AHaloFloodFanGame01GameMode::AHaloFloodFanGame01GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	
}

void AHaloFloodFanGame01GameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> OutActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHaloSpawner::StaticClass(), OutActors);
	for (AActor* a : OutActors)
	{
		Spawners.Add(Cast<AHaloSpawner>(a));
	}
	SpawnWave(Wave);
}

void AHaloFloodFanGame01GameMode::OnEnemyKilled()
{
	CurrentEnemyCount--;
	UE_LOG(LogTemp, Warning, TEXT("Enemies: %d"), CurrentEnemyCount);
	if (CurrentEnemyCount <= 0)
		WaveFinished();
}

void AHaloFloodFanGame01GameMode::WaveFinished()
{
	if (curWave == maxWave)
	{
		if (curSet == maxSet)
		{
			GameFinished();
			return;
		} else
		{
			curSet++;
			curWave = 0;
		}
	}
	curWave++;
	SpawnWave(Wave);
}

int AHaloFloodFanGame01GameMode::GetCurrentWave()
{
	return curWave;
}

int AHaloFloodFanGame01GameMode::GetCurrentSet()
{
	return curSet;
}

void AHaloFloodFanGame01GameMode::SpawnWave(TArray<FWaveStruct> WaveToSpawn)
{
	TArray<AHaloSpawner*> AvailableSpawners = Spawners;
	UE_LOG(LogTemp, Warning, TEXT("Spawning wave!"));
	for (auto WaveSquad : WaveToSpawn)
	{
		AHaloSpawner* Spawner = AvailableSpawners[UKismetMathLibrary::RandomInteger(AvailableSpawners.Num())];
		TArray<ABaseCharacter*> SpawnedChars = Spawner->SpawnSquad(WaveSquad.Squad);
		for (auto SpawnedChar : SpawnedChars)
		{
			if (SpawnedChar)
			{
				SpawnedChar->OnKilled.AddDynamic(this, &AHaloFloodFanGame01GameMode::OnEnemyKilled);
				CurrentEnemyCount++;
			}
		}
		AvailableSpawners.Remove(Spawner);
		if (AvailableSpawners.Num()==0) AvailableSpawners = Spawners;
	}
}

void AHaloFloodFanGame01GameMode::GameFinished()
{
	ResetLevel();
}
