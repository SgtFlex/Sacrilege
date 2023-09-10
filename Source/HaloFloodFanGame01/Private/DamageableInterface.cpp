// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageableInterface.h"

#include "HealthComponent.h"
#include "Core/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

// Add default functionality here for any IDamageableInterface functions that are not pure virtual.

float IDamageableInterface::CustomOnTakeAnyDamage(float DamageAmount, FVector Force,
	AController* EventInstigator, AActor* DamageCauser)
{
	return 0;
}

float IDamageableInterface::CustomTakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
                                             AController* EventInstigator, AActor* DamageCauser)
{
	CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
	if (GetHealthComponent())
	{
		GetHealthComponent()->TakeDamage(DamageAmount, Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
	}
	return 0;
}

float IDamageableInterface::CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force,
                                            AController* EventInstigator, AActor* DamageCauser)
{
	//TakeDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, FDamageEvent(UDamageType::StaticClass()), EventInstigator, DamageCauser);
	CustomOnTakeAnyDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, EventInstigator, DamageCauser);
	if (GetHealthComponent())
	{
		GetHealthComponent()->TakeDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	}
	return 0;
}

float IDamageableInterface::CustomTakeRadialDamage(float Force,
	FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//FDamageEvent DamageEvent;
	AActor* HitActor = Cast<AActor>(this);
	FVector Direction = (HitActor->GetActorLocation() - RadialDamageEvent.Origin);
	Direction.Normalize();
	CustomOnTakeAnyDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, EventInstigator, DamageCauser);
	//if (EventInstigator) UE_LOG(LogTemp, Warning, TEXT("Enemy took radial damage. EventInstigator: %s"), *EventInstigator->GetActorLabel());
	//TakeDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, FDamageEvent(UDamageType::StaticClass()));
	if (GetHealthComponent())
	{
		GetHealthComponent()->TakeDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
		//TakeDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, FDamageEvent(UDamageType::StaticClass()));
	}
		
	
	return 0;
}
//
// void IDamageableInterface::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName)
// {
// }

UHealthComponent* IDamageableInterface::GetHealthComponent()
{
	return nullptr;
}
