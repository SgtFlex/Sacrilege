// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

#include "PlayerHUD.h"
#include "Core/CharacterBase.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

APlayerControllerBase::APlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	TeamId = FGenericTeamId(TeamNumber);

	
}

void APlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// check(this);
	// if (PlayerHUDClass)
	// {
	// 	auto* PlayerHUD = CreateWidget<UUserWidget>(this, PlayerHUDClass);
	// 	//PlayerHUD->PlayerCharacter = Cast<AHaloFloodFanGame01Character>(InPawn);
	// 	PlayerHUD->AddToPlayerScreen();
	// }
}

FGenericTeamId APlayerControllerBase::GetGenericTeamId() const
{
	return TeamId;
}
