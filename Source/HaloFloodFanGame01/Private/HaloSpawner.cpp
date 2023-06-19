// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloSpawner.h"
#include "Core/BaseCharacter.h"

#include "Components/BoxComponent.h"
#include "Core/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHaloSpawner::AHaloSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxVolume"));

}

// Called when the game starts or when spawned
void AHaloSpawner::BeginPlay()
{
	Super::BeginPlay();

	
	
}

// Called every frame
void AHaloSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<ABaseCharacter*> AHaloSpawner::SpawnSquad(TMap<TSubclassOf<ABaseCharacter>, int> SquadToSpawn)
{
	if (!SpawnedChars.IsEmpty())
	{
		for (auto SpawnedChar : SpawnedChars)
		{
			if (SpawnedChar) SpawnedChar->Destroy();
		}
	}
	SpawnedChars.Empty();
	FVector BoxExtents = Box->GetScaledBoxExtent();
	BoxExtents[2] = 0;
	
	for (auto elem : SquadToSpawn)
	{
		for (int i = 0; i < elem.Value; ++i)
		{
			FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
			FRotator Rot = FRotator(0,0,0);
			ABaseCharacter* Char = GetWorld()->SpawnActor<ABaseCharacter>(elem.Key, Loc, Rot);
			SpawnedChars.Add(Char);
			if (Char && SmartObj)
			{
				Char->SmartObject = SmartObj;
			}
			
		}
	}

	return SpawnedChars;
}

