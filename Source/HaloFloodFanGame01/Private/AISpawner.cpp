// Fill out your copyright notice in the Description page of Project Settings.


#include "AISpawner.h"

#include "HealthComponent.h"
#include "Core/CharacterBase.h"

#include "Components/BoxComponent.h"
#include "Core/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AAISpawner::AAISpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxVolume"));

}

// Called when the game starts or when spawned
void AAISpawner::BeginPlay()
{
	Super::BeginPlay();

	
	
}

void AAISpawner::OnUnitKilled(UHealthComponent* HealthComponent)
{
	if (HealthComponent->GetHealth() <= 0)
	{
		SpawnedChars.Remove(Cast<ACharacterBase>(HealthComponent->GetOwner()));
	}
	if (SpawnedChars.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawner available!"));
		OnAvailable.Broadcast(this);
	}
}

// Called every frame
void AAISpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<ACharacterBase*> AAISpawner::SpawnSquad(TMap<TSubclassOf<ACharacterBase>, int> SquadToSpawn)
{
	SpawnedChars.Empty();
	FVector BoxExtents = Box->GetScaledBoxExtent();
	BoxExtents[2] = 0;
	
	for (auto elem : SquadToSpawn)
	{
		for (int i = 0; i < elem.Value; ++i)
		{
			FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
			FRotator Rot = FRotator(0,0,0);
			ACharacterBase* Char = GetWorld()->SpawnActor<ACharacterBase>(elem.Key, Loc, Rot);
			if (Char)
			{
				Char->GetHealthComponent()->OnHealthUpdate.AddDynamic(this, &AAISpawner::OnUnitKilled);
				SpawnedChars.Add(Char);
				if (SmartObj)
				{
					Char->SmartObject = SmartObj;
				}
			}
			
			
		}
	}

	return SpawnedChars;
}

