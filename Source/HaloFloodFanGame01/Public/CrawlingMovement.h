// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CrawlingMovement.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UCrawlingMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;
	
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;

	virtual void RequestPathMove(const FVector& MoveInput) override;

	virtual void StopActiveMovement() override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AccelerationSpeed = 50;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BrakingSpeed = 50;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxSpeed = 50;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetDirection;
private:
	UPROPERTY()
	FVector CurrentVelocity;

	

	UPROPERTY()
	float CurrentSpeed;

	UPROPERTY()
	float TargetSpeed;
	
	UPROPERTY()
	UPrimitiveComponent* Root;
};
