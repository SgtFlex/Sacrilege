// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherActor.h"

#include "Components/SkyLightComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Dataflow/DataflowNodeParameters.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeatherActor::AWeatherActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>("Directional Light");
	DirectionalLight->SetupAttachment(RootComponent);
	SkyLight = CreateDefaultSubobject<USkyLightComponent>("Sky Light");
	SkyLight->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeatherActor::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		StartTime = RandomStartTime ? FMath::RandRange(0, 360) : GetActorRotation().GetDenormalized().Pitch;
		CurrentPitch = StartTime;
	}
}

// Called every frame
void AWeatherActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		CurrentPitch = StartTime + ((GetWorld()->TimeSeconds*0.1) * TimeScale);
		UKismetMathLibrary::FMod(StartTime + ((GetWorld()->TimeSeconds*0.1) * TimeScale), 360.0f, CurrentPitch); //Do one full rotation every full hour
	}
	
	SetActorRotation(FRotator(CurrentPitch,0, 0));
	DirectionalLight->SetLightColor(DirectionalLightColorCurve->GetLinearColorValue(CurrentPitch));
}

void AWeatherActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeatherActor, CurrentPitch);
}

