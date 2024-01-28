// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherActor.h"

// Sets default values
AWeatherActor::AWeatherActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeatherActor::BeginPlay()
{
	Super::BeginPlay();

	
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>("Directional Light");
	SetRootComponent(DirectionalLight);
}

// Called every frame
void AWeatherActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	DirectionalLight->SetWorldRotation(FRotator((GetWorld()->TimeSeconds/60) * 360,0,0));

}

