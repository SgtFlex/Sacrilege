// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DirectionalLightComponent.h"
#include "GameFramework/Actor.h"
#include "WeatherActor.generated.h"

UCLASS()
class HALOFLOODFANGAME01_API AWeatherActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDirectionalLightComponent* DirectionalLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkyLightComponent* SkyLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeScale = 1;
	
	UPROPERTY()
	float Time;

	float StartTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RandomStartTime = false;

	float DirectLightIntensity;

	float SkyLightIntensity;
};
