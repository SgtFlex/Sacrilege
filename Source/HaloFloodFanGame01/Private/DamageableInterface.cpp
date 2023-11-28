// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageableInterface.h"

#include "HealthComponent.h"
#include "Core/CharacterBase.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

// Add default functionality here for any IDamageableInterface functions that are not pure virtual.

float IDamageableInterface::CustomOnTakeAnyDamage_Implementation(float DamageAmount, FVector Force,
	AController* EventInstigator, AActor* DamageCauser)
{
	return DamageAmount;
}

float IDamageableInterface::CustomTakeDamage_Implementation(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
                                             AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(DamageAmount, Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

float IDamageableInterface::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force,
                                            AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
}

float IDamageableInterface::CustomTakeRadialDamage_Implementation(float Force,
	FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(RadialDamageEvent.Params.BaseDamage, (Cast<AActor>(this)->GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal() * Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

UHealthComponent* IDamageableInterface::GetHealthComponent_Implementation()
{
	return Cast<AActor>(this)->FindComponentByClass<UHealthComponent>();
}

float IDamageableInterface::ChangeHealth(float Damage, FVector Force, FVector HitLocation, FName HitBoneName,
	AController* EventInstigator, AActor* DamageCauser, bool bIgnoreShields, bool bIgnoreHealthArmor,
	bool bIgnoreShieldArmor)
{
	Execute_CustomOnTakeAnyDamage(Cast<AActor>(this), Damage, Force, EventInstigator, DamageCauser);
	if (Execute_GetHealthComponent(Cast<AActor>(this)))
		return Execute_GetHealthComponent(Cast<AActor>(this))->TakeDamage(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser, false, false, false);
	return Damage;
}
