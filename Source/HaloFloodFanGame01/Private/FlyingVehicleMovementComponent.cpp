// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingVehicleMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void UFlyingVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	PrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
}

void UFlyingVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(PrimitiveComponent)) return;
	CurrentVelocity = PrimitiveComponent->GetPhysicsLinearVelocity();
	CurrentDirection = GetOwner()->GetActorForwardVector();
	CurrentRotation = GetOwner()->GetActorRotation();
	if (PrimitiveComponent->IsSimulatingPhysics())
	{
		if (TargetDirection.Length() > 0 && CurrentVelocity.Length() < MaxSpeed)
			Accelerate();
		else
			Decelerate();
		TurnToTargetRotation();
	}
}

void UFlyingVehicleMovementComponent::Accelerate()
{
	const FVector Force = TargetDirection * ThrustForce;
	PrimitiveComponent->AddForce(Force, NAME_None, true);
}

void UFlyingVehicleMovementComponent::Decelerate()
{
	PrimitiveComponent->AddForce(PrimitiveComponent->GetPhysicsLinearVelocity() * -1, NAME_None, true);
	//TargetDirection = (PrimitiveComponent->GetPhysicsLinearVelocity().GetSafeNormal() * -1).ForwardVector;
}

//@TODO Try using vectors purely instead of rotators? could be what's causing issues when aiming certain directions
void UFlyingVehicleMovementComponent::TurnToTargetRotation()
{
	CurrentDirection;
	FRotator AngleDifference = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
	const FVector CurrentAngularVelocity = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	const FVector TorqueToAdd = (FVector(0, 0, DeltaRotation.Yaw) - (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;

	const FVector AngularVelocity = ((FVector(AngleDifference.Roll, 0, AngleDifference.Yaw))
		- (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;
	
	PrimitiveComponent->AddTorqueInDegrees(AngularVelocity, NAME_None, true);
}

void UFlyingVehicleMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	const FVector VehicleLocation = GetOwner()->GetActorLocation();
	const FVector Destination = VehicleLocation + MoveVelocity * GetWorld()->GetDeltaSeconds();
	DrawDebugSphere(GetWorld(), Destination, 100, 5, FColor::Blue, false, 1);
	DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Destination, FColor::Red, false, 0.1f, 0, 3.f);
	DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), VehicleLocation + (MoveVelocity - (PrimitiveComponent->GetPhysicsLinearVelocity()*20)).GetSafeNormal() * 500, FColor::Green, false, 0.1f, 0, 3.f);
	TargetDirection = (MoveVelocity - PrimitiveComponent->GetPhysicsLinearVelocity()*30).GetSafeNormal();
}

void UFlyingVehicleMovementComponent::StopActiveMovement()
{
	Super::StopActiveMovement();
	TargetDirection = FVector(0,0,0);
}

void UFlyingVehicleMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFlyingVehicleMovementComponent, TargetRotation);
	DOREPLIFETIME(UFlyingVehicleMovementComponent, TargetDirection);
}
