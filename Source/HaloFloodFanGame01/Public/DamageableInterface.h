// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HALOFLOODFANGAME01_API IDamageableInterface
{
	GENERATED_BODY()

public:
	virtual float TakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	virtual float TakePointDamage(float DamageAmount, FVector Force, FPointDamageEvent const& PointDamageEvent, AController*  = nullptr, AActor* DamageCauser = nullptr);

	virtual float TakeRadialDamage(float DamageAmount, FVector Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	virtual void HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
