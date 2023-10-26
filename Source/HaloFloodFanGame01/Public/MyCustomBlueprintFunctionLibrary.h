// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyCustomBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UMyCustomBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void Ignite(UPrimitiveComponent* Component, float DamagePerSecond, float Duration);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"))
	static void FireHitScanBullet(FHitResult& Hit, const UObject* WorldContextObject, TArray<AActor*>& ActorsToIgnore, FVector StartLocation = FVector(0,0,0), FVector Direction = FVector(0,0,0), float Range = 5000, UCurveFloat* FalloffCurve = nullptr, float Damage = 5, float Force = 100, AActor* DamageCauser = nullptr, AController* EventInstigator = nullptr);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"))
	static AActor* FireProjectile(const UObject* WorldContextObject, FVector StartLocation, FVector Direction, TSubclassOf<AActor> ActorToSpawn);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"))
	static TArray<AActor*> FireExplosion(const UObject* WorldContextObject, FVector Location, FRadialDamageEvent RadialDamageEvent);
};
