// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherActor.h"

#include "Components/SkyLightComponent.h"
#include "Dataflow/DataflowNodeParameters.h"

// Sets default values
AWeatherActor::AWeatherActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>("Directional Light");

	SkyLight = CreateDefaultSubobject<USkyLightComponent>("Sky Light");
}

// Called when the game starts or when spawned
void AWeatherActor::BeginPlay()
{
	Super::BeginPlay();
	
	StartTime = RandomStartTime ? FMath::RandRange(0, 360) : GetActorRotation().Pitch;

	DirectLightIntensity = DirectionalLight->Intensity;
	SkyLightIntensity = SkyLight->Intensity;
}

// Called every frame
void AWeatherActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float PitchTime = StartTime + ((GetWorld()->TimeSeconds*0.1) * TimeScale); //Do one full rotation every full hour

	SetActorRotation(FRotator(PitchTime,0, 0));

	if (GetActorRotation().Pitch > 0 && GetActorRotation().Pitch < 180)
	{
		DirectionalLight->SetIntensity(0);
		SkyLight->SetIntensity(0);
	} else
	{
		DirectionalLight->SetIntensity(DirectLightIntensity);
		SkyLight->SetIntensity(SkyLightIntensity);
	}
	//DirectionalLight->SetIntensity(GetActorRotation().Pitch);

}

