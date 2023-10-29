// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
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
	UFUNCTION()
	virtual float CustomOnTakeAnyDamage(float DamageAmount, FVector Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	
	UFUNCTION(BlueprintCallable)
	virtual float CustomTakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual float CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual float CustomTakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual UHealthComponent* GetHealthComponent();

	UFUNCTION(BlueprintCallable)
	virtual float ChangeHealth(float Damage, FVector Force = FVector(0,0,0), FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShields = false, bool bIgnoreHealthArmor = false, bool bIgnoreShieldArmor = false);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
