// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01GameMode.h"

#include "HaloSpawner.h"
#include "Components/AudioComponent.h"
#include "Core/BaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet\GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AHaloFloodFanGame01GameMode::AHaloFloodFanGame01GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;
	SoundtrackComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundtrackComponent"));
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
	AvailableSpawners = Spawners;
	StartSet();
}

void AHaloFloodFanGame01GameMode::OnEnemyKilled()
{
	CurrentEnemyCount--;

	if (curWave != maxWave)
	{
		if (CurrentEnemyCount<=4)
			FinishWave();
	} else
	{
		if (CurrentEnemyCount<=0)
			FinishWave();
	}
}

void AHaloFloodFanGame01GameMode::FinishWave()
{
	MaxWavePool = MaxWavePool + 1;
	CurWavePool = MaxWavePool;
	
	
	
	
	if (curWave == maxWave)
	{
		if (curSet == maxSet)
		{
			GameFinished();
			return;
		} else
		{
			FinishSet();
		}
	} else
	{
		StartWave();
	}
}

int AHaloFloodFanGame01GameMode::GetCurrentWave()
{
	return curWave;
}

void AHaloFloodFanGame01GameMode::StartSet()
{
	curSet++;
	curWave = 0;
	StartWave();
	SoundtrackComponent->SetSound(Soundtracks[FMath::RandRange(0, Soundtracks.Num()-1)]);
	SoundtrackComponent->FadeIn(3, 0.3);
}

void AHaloFloodFanGame01GameMode::FinishSet()
{
	MaxSquadCost = (MaxSquadCost + 1) * 2;
	GetSoundtrackComponent()->FadeOut(10, 0);
	GetWorldTimerManager().SetTimer(SetFinishDelayTimer, this, &AHaloFloodFanGame01GameMode::StartSet, 10);
}

void AHaloFloodFanGame01GameMode::StartWave()
{
	curWave++;
	SquadsToSpawn.Append(CalculateWave());
	SpawnWave(SquadsToSpawn);
}

int AHaloFloodFanGame01GameMode::GetCurrentSet()
{
	return curSet;
}

TArray<FSquadStruct> AHaloFloodFanGame01GameMode::CalculateWave()
{
	
	TArray<FSquadStruct> Wave;
	TArray<FSquadStruct> AvailableSquads = SquadPool;
	
	while (CurWavePool > 0)
	{
		if (AvailableSquads.IsEmpty()) break;
		int PickedSquadIndex = FMath::RandRange(0, AvailableSquads.Num()-1);
		FSquadStruct PickedSquad = AvailableSquads[PickedSquadIndex];
		
		if (PickedSquad.Cost > CurWavePool || PickedSquad.Cost > MaxSquadCost)
		{
			AvailableSquads.RemoveAt(PickedSquadIndex);
		} else
		{
			CurWavePool -= PickedSquad.Cost;
			Wave.Add(PickedSquad);
		}
	}

	return Wave;
}

void AHaloFloodFanGame01GameMode::SpawnWave(TArray<FSquadStruct> WaveToSpawn)
{
	OnWaveStart.Broadcast(curSet, curWave);
	SquadsAtWaveStart = SquadsToSpawn;
	SquadsAtWaveStart.Append(WaveToSpawn);
	SquadsToSpawn = WaveToSpawn;
	
	//TArray<AHaloSpawner*> AvailableSpawners = Spawners;
	if (Spawners.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No spawners found"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Spawning wave!"));
	for (auto AvailableSpawner : AvailableSpawners)
	{
		TArray<ABaseCharacter*> SpawnedChars = AvailableSpawner->SpawnSquad(SquadsToSpawn[0].SquadUnits);
		{
			for (auto SpawnedChar : SpawnedChars)
			{
				if (SpawnedChar)
				{
					SpawnedChar->OnKilled.AddDynamic(this, &AHaloFloodFanGame01GameMode::OnEnemyKilled);
					CurrentEnemyCount++;
				}
			}
		}
		SquadsToSpawn.RemoveAt(0);
		if (SquadsToSpawn.IsEmpty()) break;
	}
}

void AHaloFloodFanGame01GameMode::OnSpawnerAvailable(AHaloSpawner* Spawner)
{
	if (SquadsToSpawn.IsEmpty()) return;
	TArray<ABaseCharacter*> SpawnedChars = Spawner->SpawnSquad(SquadsToSpawn[0].SquadUnits);
	{
		for (auto SpawnedChar : SpawnedChars)
		{
			if (SpawnedChar)
			{
				SpawnedChar->OnKilled.AddDynamic(this, &AHaloFloodFanGame01GameMode::OnEnemyKilled);
				CurrentEnemyCount++;
			}
		}
	}
	
}

void AHaloFloodFanGame01GameMode::GameFinished()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

UAudioComponent* AHaloFloodFanGame01GameMode::GetSoundtrackComponent()
{
	return SoundtrackComponent;
}
