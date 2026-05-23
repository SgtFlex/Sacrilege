// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "FlyingVehicleMovementComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class HALOFLOODFANGAME01_API UFlyingVehicleMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Accelerate();
	
	void Decelerate();


	UFUNCTION(BlueprintCallable)
	virtual void TurnToTargetRotation();

	void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed);


	virtual void StopActiveMovement() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector TargetDirection;

	FVector CurrentDirection;
	
	FVector CurrentVelocity;
	
	UPROPERTY(EditAnywhere)
	float ThrustForce = 1000000.0;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FQuat TargetQuat;
	
	FRotator CurrentRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueForce = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueDamping = 0.5f;
};