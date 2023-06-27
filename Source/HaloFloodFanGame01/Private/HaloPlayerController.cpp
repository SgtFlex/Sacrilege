// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloPlayerController.h"

#include "HaloHUDWidget.h"
#include "Core/BaseCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "Kismet/GameplayStatics.h"

AHaloPlayerController::AHaloPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TeamId = FGenericTeamId(1);
}

void AHaloPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
}

void AHaloPlayerController::OnPossess(APawn* InPawn)
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

FGenericTeamId AHaloPlayerController::GetGenericTeamId() const
{
	return TeamId;
}
