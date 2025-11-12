// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirefightGamemode.h"

#include "AISpawner.h"
#include "FirefightGameState.h"
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
}

void AFirefightGameMode::BeginPlay()
{
	Super::BeginPlay();

	FirefightGameState = GetGameState<AFirefightGameState>();
	SetMatchState(MatchState::InProgress);
	FirefightGameState->SetPlayerLives(PlayerLives);
}

void AFirefightGameMode::EnemyKilled_Implementation(ACharacterBase* Character, AController* EventInstigator, AActor* DamageCauser)
{
	Character->OnDestroyed.RemoveDynamic(this, &AFirefightGameMode::CharacterDestroyed);
	if (APlayerController* PlayerController = Cast<APlayerController>(EventInstigator))
	{
		AHaloPlayerState* HPS = PlayerController->GetPlayerState<AHaloPlayerState>();
		if (ScoreMap.Contains(Character->GetClass()))
		{
			if (ScoreMap[Character->GetClass()] > 0)
			{
				HPS->AddPlayerScore(ScoreMap[Character->GetClass()]);
				HPS->AddPlayerResource(ScoreMap[Character->GetClass()]);
			}
		} else
		{
			HPS->AddPlayerScore(1);
			HPS->AddPlayerResource(1);
		}
		
		
		//OnScoreUpdated.Broadcast(PlayerController, HPS->GetPlayerScore(), HPS->GetPlayerResource());
	}
	CurrentEnemyCount--;
	FirefightGameState->SetCurrentEnemyCount(CurrentEnemyCount);
	

	if (FirefightGameState->GetCurrentWave() != maxWave)
	{
		if (CurrentEnemyCount<=4)
			FinishWave();
	} else
	{
		if (CurrentEnemyCount<=0)
			FinishWave();
	}
	FirefightEnemies.Remove(Character);
	OnEnemyKilled.Broadcast(Character, EventInstigator, DamageCauser);
}

void AFirefightGameMode::FinishWave()
{
	MaxWavePool = MaxWavePool + 1;
	CurWavePool = MaxWavePool;
	
	
	
	
	if (FirefightGameState->GetCurrentWave() == maxWave)
	{
		if (FirefightGameState->GetCurrentSet() == maxSet)
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
	TArray<AActor*> OutActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAISpawner::StaticClass(), OutActors);
	for (AActor* a : OutActors)
	{
		Spawners.Add(Cast<AAISpawner>(a));
	}
	AvailableSpawners = Spawners;
	//OnGameStart.Broadcast();

	FTimerHandle SetDelayTimerHandle;
	FirefightGameState->MatchStarted();
	GetWorldTimerManager().SetTimer(SetDelayTimerHandle, this, &AFirefightGameMode::StartSet, 10.0f);
	OnPlayerCharDied.AddDynamic(this, &AFirefightGameMode::PlayerDied);
}

void AFirefightGameMode::StartSet()
{
	//GetWorld()->GetSubsystem<UNotificationSubsystem>()->PushGlobalNotification("Set start");
	FirefightGameState->SetCurrentSet(FirefightGameState->GetCurrentSet()+1);
	FirefightGameState->SetCurrentWave(0);
	//OnSetStart.Broadcast(HaloGameState->GetCurrentWave(), HaloGameState->GetCurrentWave());
	StartWave();
	
}

void AFirefightGameMode::FinishSet()
{
	MaxSquadCost = (MaxSquadCost + 1) * 2;
	FirefightGameState->SetFinished();
	GetWorldTimerManager().SetTimer(SetFinishDelayTimer, this, &AFirefightGameMode::StartSet, 10);
	OnSetEnd.Broadcast();
}

void AFirefightGameMode::StartWave()
{
	FirefightGameState->SetCurrentWave(FirefightGameState->GetCurrentWave()+1);
	SquadsToSpawn.Append(CalculateWave());
	SpawnWave(SquadsToSpawn);
	FirefightGameState->SetCurrentEnemyCount(CurrentEnemyCount);
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

	TArray<AAISpawner*> SpawnersTemp = AvailableSpawners;
	TArray<AAISpawner*> VehicleSpawners;
	for (auto Spawner : Spawners)
	{
		if (Spawner->bCanSpawnVehicles)
			VehicleSpawners.Add(Spawner);
	}
	UE_LOG(LogTemp, Warning, TEXT("Spawning wave"));
	
	while (!SquadsToSpawn.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Checking spawn conditions..."));
		//Check to see if we're spawning vehicles or not
		if (!SquadsToSpawn[0].SquadVehicles.IsEmpty())
		{
			if (!VehicleSpawners.IsEmpty())
			{
				VehicleSpawners[0]->SpawnSquad(SquadsToSpawn[0].SquadUnits, SquadsToSpawn[0].SquadVehicles);
				VehicleSpawners.RemoveAt(0);
				UE_LOG(LogTemp, Warning, TEXT("Spawning vehicle squad"));
			} else UE_LOG(LogTemp, Warning, TEXT("No vehicle spawners found"));
		} else
		{
			if (!SpawnersTemp.IsEmpty())
			{
				SpawnersTemp[0]->SpawnSquad(SquadsToSpawn[0].SquadUnits, SquadsToSpawn[0].SquadVehicles);
                SpawnersTemp.RemoveAt(0);
                UE_LOG(LogTemp, Warning, TEXT("Spawning infantry only squad"));
			} else UE_LOG(LogTemp, Warning, TEXT("No spawners found"));
		}
		SquadsToSpawn.RemoveAt(0);
	}
	UE_LOG(LogTemp, Warning, TEXT("Finished spawning wave"));
}

void AFirefightGameMode::CharacterDestroyed(AActor* DestroyedActor)
{
	EnemyKilled(Cast<ACharacterBase>(DestroyedActor), nullptr, nullptr);
}

void AFirefightGameMode::ManageCharacter(ACharacterBase* Character)
{
	Character->OnKilled.AddDynamic(this, &AFirefightGameMode::EnemyKilled);
	//Had to disable, doubles the speed of progressing waves
	Character->OnDestroyed.AddDynamic(this, &AFirefightGameMode::CharacterDestroyed);
	FirefightEnemies.Add(Character);
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

	if (FirefightGameState->GetCurrentPlayerLives() > 0)
	{
		FirefightGameState->SetPlayerLives(FirefightGameState->GetCurrentPlayerLives() - 1);
		StartRespawnProcess(PlayerController, PlayerCharacter);
	} else
	{
		EndGame();
	}
}

bool AFirefightGameMode::SpawnPlayer(APlayerControllerBase* PlayerController, uint8 Team, TSubclassOf<AGunBase> PrimaryWeaponClass, TSubclassOf<AGunBase> SecondaryWeaponClass, TSubclassOf<ACharacterBase> CharacterClass)
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
	FirefightGameState->MatchEnded();
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
