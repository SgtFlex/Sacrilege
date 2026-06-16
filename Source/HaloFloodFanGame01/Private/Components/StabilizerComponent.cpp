// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StabilizerComponent.h"

// Sets default values for this component's properties
UStabilizerComponent::UStabilizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	
	// ...
}


// Called when the game starts
void UStabilizerComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if (PrimitiveComponent)
	{
		CurrentOrientation = PrimitiveComponent->GetComponentQuat();
		TargetOrientation = CurrentOrientation;
	}
	// ...
	
}


// Called every frame
void UStabilizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PrimitiveComponent && PrimitiveComponent->IsSimulatingPhysics())
		TurnToTargetOrientation();
	// ...
}

void UStabilizerComponent::TurnToTargetOrientation()
{
	CurrentOrientation = PrimitiveComponent->GetComponentQuat();

	const FQuat AngleDifference = (TargetOrientation * CurrentOrientation.Inverse()).GetNormalized();
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Cyan, FString::Printf(TEXT("%s"), *AngleDifference.ToString()));

	const FVector CurrentAngularVelocity = PrimitiveComponent->GetPhysicsAngularVelocityInDegrees();
	FVector NewAngularVelocity = ((FVector(AngleDifference.X, AngleDifference.Y, AngleDifference.Z)*TorqueForce)
		- (CurrentAngularVelocity * Damping)) * OverallStrength;
	
	NewAngularVelocity = FVector(bStabilizeRoll ? NewAngularVelocity.X : 0, bStabilizePitch ? NewAngularVelocity.Y : 0, bStabilizeYaw ? NewAngularVelocity.Z : 0);
	
	PrimitiveComponent->AddTorqueInDegrees(NewAngularVelocity, NAME_None, true);
}

void UStabilizerComponent::SetTargetOrientation(FQuat NewOrientation)
{
	TargetOrientation = NewOrientation;
}

void UStabilizerComponent::GetTargetOrientation(FQuat& OutOrientation)
{
	OutOrientation = TargetOrientation;
}

