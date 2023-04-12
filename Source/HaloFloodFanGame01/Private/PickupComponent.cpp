// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupComponent.h"

#include "PickupInterface.h"
#include "Components/BoxComponent.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"

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
	IPickupInterface* Owner = Cast<IPickupInterface>(GetOwner());
	if (AHaloFloodFanGame01Character* PlayerChar = Cast<AHaloFloodFanGame01Character>(OtherActor))
		Owner->Pickup(PlayerChar);
}

void UPickupComponent::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
}

