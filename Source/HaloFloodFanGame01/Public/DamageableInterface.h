// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
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
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float CustomTakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	virtual float CustomTakeDamage_Implementation(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	virtual float CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float CustomTakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	virtual float CustomTakeRadialDamage_Implementation(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UHealthComponent* GetHealthComponent();
	virtual UHealthComponent* GetHealthComponent_Implementation();

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
