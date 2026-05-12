// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "WheeledVehicleMovementComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class HALOFLOODFANGAME01_API UWheeledVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()


	virtual void RequestPathMove(const FVector& MoveInput) override;
	
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	
	virtual void StopActiveMovement() override;

	virtual void StopMovementKeepPathing() override;

	virtual void StopMovementImmediately() override;

	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TurningSensitivity = 25.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float StoppingDistance = 1000.0f;
};
