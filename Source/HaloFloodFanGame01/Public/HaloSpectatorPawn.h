// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Perception/AISightTargetInterface.h"
#include "HaloSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API AHaloSpectatorPawn : public ASpectatorPawn, public IAISightTargetInterface
{
	GENERATED_BODY()

	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const override;
};
