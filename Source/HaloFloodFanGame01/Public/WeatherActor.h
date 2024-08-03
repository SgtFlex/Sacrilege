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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USceneComponent* SceneComponent;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UDirectionalLightComponent* DirectionalLight;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	USkyLightComponent* SkyLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UCurveLinearColor* DirectionalLightColorCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UCurveLinearColor* SkyLightColorCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeScale = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RandomStartTime = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentPitch;

	
private:
protected:
	UPROPERTY()
	float StartTime;
};
