// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AmmoComponent.h"



// Sets default values for this component's properties
UAmmoComponent::UAmmoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	CurrentAmmo = AmmoTotal;
	// ...
}


// Called when the game starts
void UAmmoComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAmmoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

int32 UAmmoComponent::GetAmmo()
{
	return CurrentAmmo;
}

int32 UAmmoComponent::SetAmmo(const int32 NewAmmo)
{
	CurrentAmmo = FMath::Clamp(NewAmmo, 0, AmmoTotal);
	OnAmmoChanged.Broadcast();
	return CurrentAmmo;
}

void UAmmoComponent::ResetAmmo()
{
	SetAmmo(AmmoTotal);
}

int32 UAmmoComponent::DecrementAmmo()
{
	return SetAmmo(--CurrentAmmo);
}

int32 UAmmoComponent::IncrementAmmo()
{
	return SetAmmo(++CurrentAmmo);
}

bool UAmmoComponent::HasAnyAmmo() const
{
	return CurrentAmmo > 0;
}

