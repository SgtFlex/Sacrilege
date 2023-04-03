// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageableInterface.h"

// Add default functionality here for any IDamageableInterface functions that are not pure virtual.

float IDamageableInterface::TakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	return 0;
}

float IDamageableInterface::TakePointDamage(float DamageAmount, FVector Force, FPointDamageEvent const& PointDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	return 0;
}

float IDamageableInterface::TakeRadialDamage(float DamageAmount, FVector Force,
	FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0;
}

void IDamageableInterface::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName)
{
}
