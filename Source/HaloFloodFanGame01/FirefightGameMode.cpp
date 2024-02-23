// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirefightGamemode.h"

#include "AISpawner.h"
#include "GrenadeWidget.h"
#include "GunBase.h"
#include "HaloPlayerState.h"
#include "PlayerCharacter.h"
#include "Components/AudioComponent.h"
#include "Core/CharacterBase.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/GameSession.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet\GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFirefightGameMode::AFirefightGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;
	SoundtrackComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundtrackComponent"));

	CurPlayerLives = PlayerLives;
}

void AFirefightGameMode::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> OutActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAISpawner::StaticClass(), OutActors);
	for (AActor* a : OutActors)
	{
		Spawners.Add(Cast<AAISpawner>(a));
	}
	AvailableSpawners = Spawners;
	StartSet();


	OnPlayerCharDied.AddDynamic(this, &AFirefightGameMode::PlayerDied);
	//ACharacterBase::TestDelegate.BindSP(this, &AHaloFloodFanGame01GameMode::TestFunc);
}

void AFirefightGameMode::OnEnemyKilled(ACharacterBase* Character, AController* EventInstigator, AActor* DamageCauser)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(EventInstigator))
	{
		
		PlayerScore += 1;
		PlayerResource += 1;
		OnScoreUpdated.Broadcast(PlayerController, PlayerScore, PlayerResource);
	}
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

void AFirefightGameMode::FinishWave()
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

int AFirefightGameMode::GetCurrentWave()
{
	return curWave;
}

void AFirefightGameMode::StartSet()
{
	curSet++;
	curWave = 0;
	StartWave();
	if (bEnableMusic)
	{
		SoundtrackComponent->SetSound(Soundtracks[FMath::RandRange(0, Soundtracks.Num()-1)]);
		SoundtrackComponent->FadeIn(3, 0.3);
	}
}

void AFirefightGameMode::FinishSet()
{
	MaxSquadCost = (MaxSquadCost + 1) * 2;
	if (bEnableMusic) GetSoundtrackComponent()->FadeOut(10, 0);
	GetWorldTimerManager().SetTimer(SetFinishDelayTimer, this, &AFirefightGameMode::StartSet, 10);
}

void AFirefightGameMode::StartWave()
{
	curWave++;
	SquadsToSpawn.Append(CalculateWave());
	SpawnWave(SquadsToSpawn);
}

int AFirefightGameMode::GetCurrentSet()
{
	return curSet;
}

TArray<FSquadStruct> AFirefightGameMode::CalculateWave()
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

void AFirefightGameMode::SpawnWave(TArray<FSquadStruct> WaveToSpawn)
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
		TArray<ACharacterBase*> SpawnedChars = AvailableSpawner->SpawnSquad(SquadsToSpawn[0].SquadUnits);
		{
			for (auto SpawnedChar : SpawnedChars)
			{
				if (SpawnedChar)
				{
					SpawnedChar->OnKilled.AddDynamic(this, &AFirefightGameMode::OnEnemyKilled);
					CurrentEnemyCount++;
				}
			}
		}
		SquadsToSpawn.RemoveAt(0);
		if (SquadsToSpawn.IsEmpty()) break;
	}
}

void AFirefightGameMode::OnSpawnerAvailable(AAISpawner* Spawner)
{
	if (SquadsToSpawn.IsEmpty()) return;
	TArray<ACharacterBase*> SpawnedChars = Spawner->SpawnSquad(SquadsToSpawn[0].SquadUnits);
	{
		for (auto SpawnedChar : SpawnedChars)
		{
			if (SpawnedChar)
			{
				SpawnedChar->OnKilled.AddDynamic(this, &AFirefightGameMode::OnEnemyKilled);
				CurrentEnemyCount++;
			}
		}
	}
	
}

void AFirefightGameMode::GameFinished()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

int AFirefightGameMode::GetPlayerResource(APlayerController* PlayerController)
{
	return PlayerResource;
}

int AFirefightGameMode::SetPlayerResource(APlayerController* PlayerController, int NewPlayerResource)
{
	return PlayerResource = NewPlayerResource;
}

int AFirefightGameMode::GetPlayerScore(APlayerController* PlayerController)
{
	return PlayerScore;
	//PlayerCharDied.Broadcast
}

void AFirefightGameMode::PlayerDied_Implementation(APlayerCharacter* PlayerCharacter, APlayerController* PlayerController)
{
	//UE_LOG(LogTemp, Warning, TEXT("Attempted to respawn player %s from character %s"), *PlayerCharacter->GetActorLabel(), *PlayerController->GetActorLabel());
	//FTimerDelegate TimerDelegate;
	//TimerDelegate.BindUFunction(this, FName("RespawnPlayer"), PlayerController);
	if (CurPlayerLives > 0)
	{
		CurPlayerLives--;
		//GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, TimerDelegate, RespawnTime, false);
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnTime, false);
		AddLoadoutScreen(PlayerController, RespawnTimerHandle);
		// UUserWidget* LoadoutWidget = CreateWidget<UUserWidget>(PlayerController, LoadoutScreenClass);
		// LoadoutWidget->AddToPlayerScreen();
	} else
	{
		EndGame();
	}
}

bool AFirefightGameMode::FinishSpawning(APlayerController* PlayerController, uint8 Team, TSubclassOf<AGunBase> PrimaryWeaponClass, TSubclassOf<AGunBase> SecondaryWeaponClass)
{
	AHaloPlayerState* HaloPlayerState = PlayerController->GetPlayerState<AHaloPlayerState>();
	if (HaloPlayerState)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Got player state"));
		HaloPlayerState->Team = Team;
		if (PrimaryWeaponClass)
		{
			HaloPlayerState->PrimaryWeaponClass = PrimaryWeaponClass;
		}
			
		if (SecondaryWeaponClass)
		{
			HaloPlayerState->SecondaryWeaponClass = SecondaryWeaponClass;
		}
			
	}
	if (GetWorld()->GetTimerManager().TimerExists(RespawnTimerHandle))
	{
		return false;
	} else
	{
		RespawnPlayer(PlayerController);
		return true;
	}
}

void AFirefightGameMode::RespawnPlayer(APlayerController* PlayerController)
{
	PlayerController->UnPossess();
	RestartPlayer(PlayerController);
}

void AFirefightGameMode::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Game ended"));
	//GetWorld()->GetPlay
	RestartGame();
}

UAudioComponent* AFirefightGameMode::GetSoundtrackComponent()
{
	return SoundtrackComponent;
	
}

void AFirefightGameMode::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	// start human players first
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && (PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
		{
			GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnTime, false);
			AddLoadoutScreen(PlayerController, RespawnTimerHandle);
			//RestartPlayer(PlayerController);
		}
	}

	// Make sure level streaming is up to date before triggering NotifyMatchStarted
	GEngine->BlockTillLevelStreamingCompleted(GetWorld());

	// First fire BeginPlay, if we haven't already in waiting to start match
	GetWorldSettings()->NotifyBeginPlay();

	// Then fire off match started
	GetWorldSettings()->NotifyMatchStarted();

	// if passed in bug info, send player to right location
	const FString BugLocString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugLoc"));
	const FString BugRotString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugRot"));
	if( !BugLocString.IsEmpty() || !BugRotString.IsEmpty() )
	{
		for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController &&  PlayerController->CheatManager != nullptr)
			{
				PlayerController->CheatManager->BugItGoString( BugLocString, BugRotString );
			}
		}
	}

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}
}
