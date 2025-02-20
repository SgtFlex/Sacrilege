// Fill out your copyright notice in the Description page of Project Settings.


#include "WheeledVehicleMovementComponent.h"

void UWheeledVehicleMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);

	
	SetSteeringInput(GetOwner()->GetActorForwardVector().Dot(MoveVelocity));
	SetThrottleInput(1.0f);
}

void UWheeledVehicleMovementComponent::StopActiveMovement()
{
	Super::StopActiveMovement();

	SetThrottleInput(0.0f);
}

void UWheeledVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

}
