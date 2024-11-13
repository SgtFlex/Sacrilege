// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

#include "AlertState.h"
#include "PlayerHUD.h"
#include "Core/CharacterBase.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01/FirefightGamemode.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

APlayerControllerBase::APlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(FGenericTeamId(TeamNumber));
}

void APlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// if (ACharacterBase* Char = Cast<ACharacterBase>(InPawn))
	// 	SetGenericTeamId(Char->TeamNumber);
	if (ACharacterBase* Char = Cast<ACharacterBase>(InPawn))
		Char->AlertState = EAlertState::Alerted;
	// 	SetGenericTeamId(Char->TeamNumber);
	UE_LOG(LogTemp, Warning, TEXT("Possessed"));
	// check(this);
	// if (PlayerHUDClass)
	// {
	// 	auto* PlayerHUD = CreateWidget<UUserWidget>(this, PlayerHUDClass);
	// 	//PlayerHUD->PlayerCharacter = Cast<AHaloFloodFanGame01Character>(InPawn);
	// 	PlayerHUD->AddToPlayerScreen();
	// }
}

void APlayerControllerBase::OnControlledCharacterDied(ACharacterBase* DeadCharacter, AController* Inst,
	AActor* Causer)
{
	OnPlayerDeath.Broadcast(ControlledCharacter, this);
}

void APlayerControllerBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (TeamID != NewTeamID)
	{
		TeamID = NewTeamID;
		// @todo notify perception system that a controller changed team ID
	}
}

void APlayerControllerBase::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (ACharacterBase* CharacterBase = Cast<ACharacterBase>(InPawn))
	{
		if (ControlledCharacter)
			ControlledCharacter->OnKilled.RemoveDynamic(this, &APlayerControllerBase::OnControlledCharacterDied);
		ControlledCharacter = CharacterBase;
		ControlledCharacter->OnKilled.AddUniqueDynamic(this, &APlayerControllerBase::OnControlledCharacterDied);
	}
}
