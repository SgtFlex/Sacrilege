// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloPlayerController.h"

AHaloPlayerController::AHaloPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TeamId = FGenericTeamId(1);
}

FGenericTeamId AHaloPlayerController::GetGenericTeamId() const
{
	return TeamId;
}
