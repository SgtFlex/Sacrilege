// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirefightGamemode.h"

#include "AISpawner.h"
#include "GunBase.h"
#include "HaloGameState.h"
#include "HaloPlayerState.h"
#include "NotificationSubsystem.h"
#include "PlayerControllerBase.h"
#include "VehicleBase.h"
#include "Core/CharacterBase.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/GameSession.h"
#include "Kismet\GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFirefightGameMode::AFirefightGameMode()
	: Super()
{
	
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;
	// SoundtrackComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundtrackComponent"));

	CurPlayerLives = PlayerLives;
}

void AFirefightGameMode::BeginPlay()
{
	Super::BeginPlay();

	HaloGameState = GetGameState<AHaloGameState>();
	SetMatchState(MatchState::InProgress);
}

void AFirefightGameMode::OnEnemyKilled(ACharacterBase* Character, AController* EventInstigator, AActor* DamageCauser)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(EventInstigator))
	{
		AHaloPlayerState* HPS = PlayerController->GetPlayerState<AHaloPlayerState>();
		HPS->AddPlayerScore(1);
		HPS->AddPlayerResource(1);
		
		//OnScoreUpdated.Broadcast(PlayerController, HPS->GetPlayerScore(), HPS->GetPlayerResource());
	}
	CurrentEnemyCount--;
	

	if (HaloGameState->GetCurrentWave() != maxWave)
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
	
	
	
	
	if (HaloGameState->GetCurrentWave() == maxWave)
	{
		if (HaloGameState->GetCurrentSet() == maxSet)
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

bool AFirefightGameMode::ReadyToStartMatch_Implementation()
{
	// return Super::ReadyToStartMatch_Implementation();
	return true;
}

//
// int AFirefightGameMode::GetCurrentWave()
// {
// 	return HaloGameState->curWave;
// }

void AFirefightGameMode::StartMatch()
{
	Super::StartMatch();
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Firefight");
	TArray<AActor*> OutActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAISpawner::StaticClass(), OutActors);
	for (AActor* a : OutActors)
	{
		Spawners.Add(Cast<AAISpawner>(a));
	}
	AvailableSpawners = Spawners;
	//OnGameStart.Broadcast();
	StartSet();


	OnPlayerCharDied.AddDynamic(this, &AFirefightGameMode::PlayerDied);
}

void AFirefightGameMode::StartSet()
{
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Set start");
	HaloGameState->SetCurrentSet(HaloGameState->GetCurrentSet()+1);
	HaloGameState->SetCurrentWave(0);
	//OnSetStart.Broadcast(HaloGameState->GetCurrentWave(), HaloGameState->GetCurrentWave());
	StartWave();
	
}

void AFirefightGameMode::FinishSet()
{
	MaxSquadCost = (MaxSquadCost + 1) * 2;
	GetWorldTimerManager().SetTimer(SetFinishDelayTimer, this, &AFirefightGameMode::StartSet, 10);
	//OnSetEnd.Broadcast(HaloGameState->GetCurrentWave(), HaloGameState->GetCurrentWave());
}

void AFirefightGameMode::StartWave()
{
	HaloGameState->SetCurrentWave(HaloGameState->GetCurrentWave()+1);
	SquadsToSpawn.Append(CalculateWave());
	SpawnWave(SquadsToSpawn);
	//OnWaveStart.Broadcast(HaloGameState->GetCurrentWave(), HaloGameState->GetCurrentWave());
}

// int AFirefightGameMode::GetCurrentSet()
// {
// 	return HaloGameState->curSet;
// }

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
	//OnWaveStart.Broadcast(HaloGameState->GetCurrentSet(), HaloGameState->GetCurrentWave());
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Reinforcements");
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
		AvailableSpawner->SpawnSquad(SquadsToSpawn[0].SquadUnits, SquadsToSpawn[0].SquadVehicles);
		// TArray<ACharacterBase*> SpawnedChars = AvailableSpawner->SpawnSquad(SquadsToSpawn[0].SquadUnits, false);
		// for (auto SpawnedChar : SpawnedChars)
		// {
		// 	if (SpawnedChar)
		// 	{
		// 		SpawnedChar->OnKilled.AddDynamic(this, &AFirefightGameMode::OnEnemyKilled);
		// 		CurrentEnemyCount++;
		// 	}
		// }
		SquadsToSpawn.RemoveAt(0);
		if (SquadsToSpawn.IsEmpty()) break;
	}
}

void AFirefightGameMode::ManageCharacter(ACharacterBase* Character)
{
	Character->OnKilled.AddDynamic(this, &AFirefightGameMode::OnEnemyKilled);
	CurrentEnemyCount++;
}

void AFirefightGameMode::OnSpawnerAvailable(AAISpawner* Spawner)
{
	if (SquadsToSpawn.IsEmpty()) return;
	// TArray<ACharacterBase*> SpawnedChars = Spawner->SpawnSquad(SquadsToSpawn[0].SquadUnits);
	// {
	// 	for (auto SpawnedChar : SpawnedChars)
	// 	{
	// 		if (SpawnedChar)
	// 		{
	// 			SpawnedChar->OnKilled.AddDynamic(this, &AFirefightGameMode::OnEnemyKilled);
	// 			CurrentEnemyCount++;
	// 		}
	// 	}
	// }
	
}

void AFirefightGameMode::GameFinished()
{
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Game over");
	UGameplayStatics::OpenLevel(GetWorld(), FName(UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

void AFirefightGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	if (APlayerControllerBase* PC = Cast<APlayerControllerBase>(NewPlayer))
		PC->OnPlayerDeath.AddUniqueDynamic(this, &AFirefightGameMode::PlayerDied);
}

void AFirefightGameMode::RestartPlayer(AController* NewPlayer)
{
	RestartPlayerBP(NewPlayer);
	
}

void AFirefightGameMode::PlayerDied_Implementation(ACharacterBase* PlayerCharacter, APlayerControllerBase* PlayerController)
{
	if (!PlayerController) return;

	if (CurPlayerLives > 0)
	{
		CurPlayerLives--;
		StartRespawnProcess(PlayerController, PlayerCharacter);
	} else
	{
		EndGame();
	}
}

bool AFirefightGameMode::FinishSpawning(APlayerControllerBase* PlayerController, uint8 Team, TSubclassOf<AGunBase> PrimaryWeaponClass, TSubclassOf<AGunBase> SecondaryWeaponClass, TSubclassOf<ACharacterBase> CharacterClass)
{
	AHaloPlayerState* HaloPlayerState = PlayerController->GetPlayerState<AHaloPlayerState>();
	if (HaloPlayerState)
	{
		HaloPlayerState->Team = Team;
		if (PrimaryWeaponClass)
		{
			HaloPlayerState->PrimaryWeaponClass = PrimaryWeaponClass;
		}
			
		if (SecondaryWeaponClass)
		{
			HaloPlayerState->SecondaryWeaponClass = SecondaryWeaponClass;
		}
		if (CharacterClass)
		{
			HaloPlayerState->CharacterClass = CharacterClass;
		}
	}
	if (GetWorld()->GetTimerManager().TimerExists(PlayerController->PlayerRespawnTimerHandle))
	{
		return false;
	} else
	{
		RespawnPlayer(PlayerController);
		return true;
	}
}

void AFirefightGameMode::RespawnPlayer(APlayerControllerBase* PlayerController)
{
	PlayerController->UnPossess();
	RestartPlayer(PlayerController);
	if (RespawnVehicle)
	{
		SpawnRespawnVehicle(PlayerController);
	}
}

void AFirefightGameMode::StartRespawnProcess(APlayerControllerBase* PC, ACharacterBase* PreviousCharacter)
{
	if (!PC) return;
	GetWorld()->GetTimerManager().SetTimer(PC->PlayerRespawnTimerHandle, RespawnTime, false);
	CreateSpectator(PreviousCharacter, PC, RespawnTime);
	
}

void AFirefightGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	//Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	StartRespawnProcess(Cast<APlayerControllerBase>(NewPlayer));
}

void AFirefightGameMode::EndGame()
{
	RestartGame();
	//OnGameEnd.Broadcast();
}


void AFirefightGameMode::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	// start human players first
	// for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	// {
	// 	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(Iterator->Get());
	// 	if (PlayerController && (PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
	// 	{
	// 		GetWorld()->GetTimerManager().SetTimer(PlayerController->PlayerRespawnTimerHandle, RespawnTime, false);
	// 		AddLoadoutScreen(PlayerController, RespawnTime);
	// 		//RestartPlayer(PlayerController);
	// 	}
	// }

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

void AFirefightGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	FString PlayerName = NewPlayer->GetPlayerState<APlayerState>()->GetName();
	const FString Text = FString::Printf(TEXT("%ls has joined"), *PlayerName);
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification(Text);
}

AVehicleBase* AFirefightGameMode::SpawnRespawnVehicle_Implementation(APlayerController* PlayerController)
{
	return nullptr;
	// FVector Loc = FVector(0, 0, 30000);
	// FRotator Rot = FRotator(0,0,0);
	// FActorSpawnParameters ActorSpawnParameters = FActorSpawnParameters();
	// ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// AVehicleBase* Vehicle = GetWorld()->SpawnActor(RespawnVehicle, &Loc, &Rot, ActorSpawnParameters);
	// if (Vehicle)
	// 	IInteractableInterface::Execute_OnInteract(Vehicle, Cast<ACharacterBase>(PlayerController->GetPawn()));
	//
	// return Vehicle;
}
