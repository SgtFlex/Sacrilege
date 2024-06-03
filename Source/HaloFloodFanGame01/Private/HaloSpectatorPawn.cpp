// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloSpectatorPawn.h"

bool AHaloSpectatorPawn::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation,
	int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible,
	int32* UserData) const
{
	//return IAISightTargetInterface::CanBeSeenFrom(ObserverLocation, OutSeenLocation, NumberOfLoSChecksPerformed,OutSightStrength, IgnoreActor, bWasVisible, UserData);
	return false;
}
