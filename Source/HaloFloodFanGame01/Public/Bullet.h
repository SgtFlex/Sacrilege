// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Bullet.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class HALOFLOODFANGAME01_API UBullet : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Force = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<AActor>> SurfaceResponsesFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* FalloffCurve;
};
