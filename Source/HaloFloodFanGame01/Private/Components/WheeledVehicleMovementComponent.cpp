// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WheeledVehicleMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// void UWheeledVehicleMovementComponent::RequestPathMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
// {
// 	//Super::RequestPathMove(MoveVelocity);
//
//
// 	//ChaosVehicleMovementComponent->SetSteeringInput(GetOwner()->GetActorForwardVector().Dot(MoveVelocity));
// 	SetBrakeInput(0.0f);
// 	SetThrottleInput(1.0f);
// }

void UWheeledVehicleMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	//Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	//UE_LOG(LogTemp, Warning, TEXT("%s starting request move"), *GetOwner()->GetActorLabel())	
	
	const FVector Position = GetOwner()->GetActorLocation();
	const FVector Destination = Position + MoveVelocity * GetWorld()->GetDeltaSeconds();
	const float DotProduct = GetOwner()->GetActorForwardVector().Dot(MoveVelocity.GetSafeNormal());
	const FVector CrossProduct = FVector::CrossProduct(GetOwner()->GetActorForwardVector().GetSafeNormal(), MoveVelocity.GetSafeNormal());
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	if (CrossProduct.Z < 0) AngleDegrees *= -1.0f;
	const float SteerValue = AngleDegrees / TurningSensitivity;
	SetSteeringInput(SteerValue);
	//UE_LOG(LogTemp, Warning, TEXT("%f dot steer length %f"), AngleDegrees, MoveVelocity.Length())
	
	
	if (FMath::Abs(AngleDegrees) > 90 && (Destination - Position).Length() < 1500)
	{
		SetTargetGear(-1, true);
		SetBrakeInput(0.0f);
		SetThrottleInput((Destination - Position).Length()/StoppingDistance);
	} else
	{
		SetTargetGear(1, true);
		SetBrakeInput(0.0f);
		SetThrottleInput(-1.0f);
	}
	
	//UKismetSystemLibrary::DrawDebugArrow(GetWorld(), Position, Destination, 30, FLinearColor::Blue, 1, 10);
}

void UWheeledVehicleMovementComponent::StopActiveMovement()
{
	//Super::StopActiveMovement();

	SetThrottleInput(0.0f);
	SetBrakeInput(1.0f);
}

void UWheeledVehicleMovementComponent::StopMovementImmediately()
{
	SetThrottleInput(0.0f);
	SetBrakeInput(1.0f);
}


void UWheeledVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}
