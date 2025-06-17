// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

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
		if (bThrust)
		{
			if (TargetDirection.Length() > 0 && CurrentVelocity.Length() < MaxSpeed)
				Accelerate();
			else
				Decelerate();
		}
		if (bTorque)
			TurnToTargetRotation();
	}
}

void UHoverMovementComponent::Accelerate()
{
	const FVector Force = TargetDirection * ThrustForce;
	PrimitiveComponent->AddForce(Force, NAME_None, true);
}

void UHoverMovementComponent::Decelerate()
{
	PrimitiveComponent->AddForce(PrimitiveComponent->GetPhysicsLinearVelocity() * -1, NAME_None, true);
}

void UHoverMovementComponent::TurnToTargetRotation()
{
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
	const FVector CurrentAngularVelocity = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	const FVector TorqueToAdd = (FVector(DeltaRotation.Roll, DeltaRotation.Pitch, DeltaRotation.Yaw) - (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;
	PrimitiveComponent->AddTorqueInDegrees(TorqueToAdd, NAME_None, true);
}

void UHoverMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	//FVector VehicleLocation = GetOwner()->GetActorLocation();
	//FVector Destination = VehicleLocation + MoveVelocity * GetWorld()->GetDeltaSeconds();
	
	//DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Destination, FColor::Red, false, 1.f, 0, 3.f);
	//DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), VehicleLocation + (MoveVelocity - (PrimitiveComponent->GetPhysicsLinearVelocity()*20)).GetSafeNormal() * 500, FColor::Green, false, 1.f, 0, 3.f);
	
	TargetDirection = (MoveVelocity - PrimitiveComponent->GetPhysicsLinearVelocity()*30).GetSafeNormal();
	TargetDirection.Z = PrimitiveComponent->GetForwardVector().Z;
	//DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + (TargetDirection.GetSafeNormal() * 500), 50, FColor::Red, false, 1);
	//TargetDirection = (MoveVelocity - PrimitiveComponent->GetPhysicsLinearVelocity()*30).GetSafeNormal();
}

void UHoverMovementComponent::StopActiveMovement()
{
	Super::StopActiveMovement();
	TargetDirection = FVector(0,0,0);
}

void UHoverMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHoverMovementComponent, TargetRotation);
	DOREPLIFETIME(UHoverMovementComponent, TargetDirection);
	DOREPLIFETIME(UHoverMovementComponent, ThrustForce);
	DOREPLIFETIME(UHoverMovementComponent, MaxSpeed);
}
