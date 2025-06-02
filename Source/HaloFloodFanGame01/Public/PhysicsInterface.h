// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PhysicsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPhysicsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HALOFLOODFANGAME01_API IPhysicsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyPhysicsImpulse(FVector Force, FVector Location, FName BoneName = NAME_None);
};
