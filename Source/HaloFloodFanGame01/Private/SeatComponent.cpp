// Fill out your copyright notice in the Description page of Project Settings.


#include "SeatComponent.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"

// Sets default values for this component's properties
USeatComponent::USeatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	InteractBox = CreateDefaultSubobject<UBoxComponent>("Interact Box");
	// ...
}


// Called when the game starts
void USeatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USeatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USeatComponent::OnInteract_Implementation(APlayerCharacter* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	EnterSeat(Character);
}

void USeatComponent::EnterSeat(ACharacterBase* Character)
{
	Character->GetCapsuleComponent()->SetSimulatePhysics(false);
	Character->SetActorTransform(GetComponentTransform());
}

void USeatComponent::ExitSeat(ACharacterBase* Character)
{
	Character->GetCapsuleComponent()->SetSimulatePhysics(true);
	Character->SetActorTransform(ExitPosition);
}

