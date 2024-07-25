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
	CurrentForward = PrimitiveComponent->GetForwardVector();
	if (PrimitiveComponent->IsSimulatingPhysics())
	{
		ThrustToTargetSpeed();
		TorqueToTargetRotation();
	}
}

void UHoverMovementComponent::ThrustToTargetSpeed()
{
	//if (PrimitiveComponent->GetComponentVelocity().Length() < MaxSpeed)
	PrimitiveComponent->AddForce((PrimitiveComponent->GetComponentRotation().RotateVector(TargetForceLocal) * ThrustForce), NAME_None, true);
}

//@TODO Try using vectors purely instead of rotators? could be what's causing issues when aiming certain directions
void UHoverMovementComponent::TorqueToTargetRotation()
{
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
	const FVector CurrentAngularVelocity = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	
	const FVector TorqueToAdd = (FVector(DeltaRotation.Roll, DeltaRotation.Pitch, DeltaRotation.Yaw) - (CurrentAngularVelocity * TorqueDamping)) * TorqueForce;
	//GEngine->AddOnScreenDebugMessage(-1, .1, FColor::Cyan, TorqueToAdd.ToString());
	//PrimitiveComponent->AddTorqueInDegrees(TorqueToAdd, NAME_None, true);
	PrimitiveComponent->AddTorqueInDegrees(TorqueToAdd, NAME_None, true);
	//PrimitiveComponent->AddTorqueInDegrees((FVector(TargetForward-CurrentForward).GetSafeNormal()) * TorqueForce, NAME_None, true);

}

void UHoverMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHoverMovementComponent, TargetRotation);
	DOREPLIFETIME(UHoverMovementComponent, TargetForceLocal);
}
