// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CrawlingMovement.h"

void UCrawlingMovement::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentVelocity = Root->GetPhysicsLinearVelocity();
	if (TargetSpeed > 0 && CurrentVelocity.Length() < MaxSpeed)
	{
		Root->AddForce(TargetDirection * AccelerationSpeed);
	} else
	{
		Root->AddForce(CurrentVelocity.GetSafeNormal() * -BrakingSpeed);
	}
	
}

void UCrawlingMovement::BeginPlay()
{
	Super::BeginPlay();

	Root = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
}

void UCrawlingMovement::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	TargetSpeed = MaxSpeed;
	TargetDirection = MoveVelocity;
}

void UCrawlingMovement::RequestPathMove(const FVector& MoveInput)
{
	Super::RequestPathMove(MoveInput);
	TargetSpeed = MaxSpeed;
	TargetDirection = MoveInput;
}

void UCrawlingMovement::StopActiveMovement()
{
	Super::StopActiveMovement();

	TargetSpeed = 0;
}
