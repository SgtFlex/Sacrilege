// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PickupComponent.h"

#include "Interfaces/PickupInterface.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"

// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	// ...
}


// Called when the game starts
void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UPickupComponent::BeginOverlap);
	// ...
	
}


// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPickupComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bEnabled) return;
	IPickupInterface* Owner = Cast<IPickupInterface>(GetOwner());
	if (ACharacterBase* Char = Cast<ACharacterBase>(OtherActor))
		Owner->Pickup(Char);
}

void UPickupComponent::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
}

