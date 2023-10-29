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
	return DamageAmount;
}

float IDamageableInterface::CustomTakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
                                             AController* EventInstigator, AActor* DamageCauser)
{
	CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
	if (GetHealthComponent())
		return GetHealthComponent()->TakeDamage(DamageAmount, Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
	return DamageAmount;
}

float IDamageableInterface::CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force,
                                            AController* EventInstigator, AActor* DamageCauser)
{
	CustomOnTakeAnyDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, EventInstigator, DamageCauser);
	if (GetHealthComponent())
		return GetHealthComponent()->TakeDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	return PointDamageEvent.Damage;
}

float IDamageableInterface::CustomTakeRadialDamage(float Force,
	FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	AActor* HitActor = Cast<AActor>(this);
	FVector Direction = (HitActor->GetActorLocation() - RadialDamageEvent.Origin);
	Direction.Normalize();
	CustomOnTakeAnyDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, EventInstigator, DamageCauser);
	if (GetHealthComponent())
		return GetHealthComponent()->TakeDamage(RadialDamageEvent.Params.BaseDamage, Direction * Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
	return RadialDamageEvent.Params.BaseDamage;
}

UHealthComponent* IDamageableInterface::GetHealthComponent()
{
	return nullptr;
}
