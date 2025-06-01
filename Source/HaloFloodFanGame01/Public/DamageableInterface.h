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
	UFUNCTION()
	virtual float CustomOnTakeAnyDamage(float DamageAmount, FVector Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeDamage(float Damage, FVector Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakePointDamage(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeRadialDamage(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UHealthComponent* GetHealthComponent();

	float ChangeHealth(AActor* Actor, float Damage, FVector Force = FVector(0,0,0), FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShields = false, bool bIgnoreHealthArmor = false, bool bIgnoreShieldArmor = false);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
