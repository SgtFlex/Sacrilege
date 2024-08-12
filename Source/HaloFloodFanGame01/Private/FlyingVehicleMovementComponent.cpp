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
	CurrentRotation = PrimitiveComponent->GetComponentRotation();
	if (PrimitiveComponent->IsSimulatingPhysics())
	{
		if (TargetDirection.Length() > 0)
			Accelerate();
		else
			Decelerate();
		TurnToTargetRotation();
	}
}

void UFlyingVehicleMovementComponent::Accelerate()
{
	//if (PrimitiveComponent->GetComponentVelocity().Length() < MaxSpeed)
	FVector LocalThrustDirection = TargetDirection;
	// if (PrimitiveComponent->GetPhysicsLinearVelocity().Length() > MaxSpeed)
	// {
	// 	LocalThrustDirection.X = 0;
	// 	//LocalThrustDirection.Y = 0;
	// }
	//GEngine->AddOnScreenDebugMessage(-1, 0.5, FColor::Green, TEXT("Accelerating!"));
	//const FVector Direction = PrimitiveComponent->GetComponentRotation().RotateVector(LocalThrustDirection);
	const FVector Force = LocalThrustDirection * ThrustForce;
	
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
	
	UE_LOG(LogTemp, Warning, TEXT("\n----------------------------------------------------------"));
	UE_LOG(LogTemp, Warning, TEXT("Current: \t %s"), *CurrentRotation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Target: \t %s"), *TargetRotation.ToString());
	
	FRotator DeltaRotation = TargetRotation - CurrentRotation;
	DeltaRotation.Normalize();
	UE_LOG(LogTemp, Warning, TEXT("Delta: \t\t %s"), *DeltaRotation.ToString());
	const FVector CurrentAngularVelocity = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	const FVector TorqueToAdd = (FVector(0, 0, DeltaRotation.Yaw) - (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;
	//const FVector TorqueToAdd = (FVector(DeltaRotation.Roll, DeltaRotation.Pitch, DeltaRotation.Yaw) - (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;
	UE_LOG(LogTemp, Warning, TEXT("Torque: \t %s"), *TorqueToAdd.ToString());
	
	PrimitiveComponent->AddTorqueInDegrees(TorqueToAdd, NAME_None, true);
}

void UFlyingVehicleMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	const FVector VehicleLocation = GetOwner()->GetActorLocation();
	const FVector Destination = VehicleLocation + MoveVelocity * GetWorld()->GetDeltaSeconds();
	
	DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Destination, FColor::Red, false, 1.f, 0, 3.f);
	DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), VehicleLocation + (MoveVelocity - (PrimitiveComponent->GetPhysicsLinearVelocity()*20)).GetSafeNormal() * 500, FColor::Green, false, 1.f, 0, 3.f);
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
