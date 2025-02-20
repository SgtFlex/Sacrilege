// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleMovementComponent.h"
#include "WheeledVehicleMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UWheeledVehicleMovementComponent : public UChaosVehicleMovementComponent
{
	GENERATED_BODY()

	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;

	virtual void StopActiveMovement() override;

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;
	
};
