// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"

void UHoverMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
}

void UHoverMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(PrimitiveComponent)) return;
	CurrentVelocity = PrimitiveComponent->GetPhysicsLinearVelocity();
	CurrentRotation = PrimitiveComponent->GetComponentRotation();
	if (PrimitiveComponent->IsSimulatingPhysics())
	{
		ThrustToTargetSpeed();
		TorqueToTargetRotation();
	}
}

void UHoverMovementComponent::ThrustToTargetSpeed()
{
	PrimitiveComponent->AddForce(PrimitiveComponent->GetComponentRotation().RotateVector(TargetForceLocal) * ThrustForce);
}

void UHoverMovementComponent::TorqueToTargetRotation()
{
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
	const FVector CurrentTorque = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	PrimitiveComponent->AddTorqueInDegrees((FVector(DeltaRotation.Roll, DeltaRotation.Pitch, DeltaRotation.Yaw) - (CurrentTorque * TorqueDamping)) * TorqueForce, NAME_None, true);
}
