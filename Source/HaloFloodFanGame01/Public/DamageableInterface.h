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
	virtual void TakeDamage(float DamageAmount);

	virtual void HealthDepleted();
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
