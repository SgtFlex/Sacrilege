// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/JetMovementComponent.h"

void UJetMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	const FVector VehicleLocation = GetOwner()->GetActorLocation();
	const FVector Destination = VehicleLocation + MoveVelocity * GetWorld()->GetDeltaSeconds();
	TargetQuat = (MoveVelocity - PrimitiveComponent->GetPhysicsLinearVelocity()*30).GetSafeNormal().ToOrientationQuat();
	TargetDirection = PrimitiveComponent->GetForwardVector();
	//DrawDebugSphere(GetWorld(), Destination, 100, 5, FColor::Blue, false, 1);
	//DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Destination, FColor::Red, false, 0.1f, 0, 3.f);
	//DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), VehicleLocation + (MoveVelocity - (PrimitiveComponent->GetPhysicsLinearVelocity()*20)).GetSafeNormal() * 500, FColor::Green, false, 0.1f, 0, 3.f);
	//TargetDirection = (MoveVelocity - PrimitiveComponent->GetPhysicsLinearVelocity()*30).GetSafeNormal();
}
