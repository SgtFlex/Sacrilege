// Fill out your copyright notice in the Description page of Project Settings.


#include "AISpawner.h"

#include "HealthComponent.h"
#include "VehicleBase.h"
#include "Core/CharacterBase.h"

#include "Components/BoxComponent.h"
#include "Core/CharacterBase.h"
#include "HaloFloodFanGame01/FirefightGamemode.h"
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
		OnAvailable.Broadcast(this);
	}
}

// Called every frame
void AAISpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAISpawner::TriggerSpawn()
{
	SpawnedChars.Empty();
	TArray<ACharacterBase*> Spawned;
	
	FVector BoxExtents = Box->GetScaledBoxExtent();
	BoxExtents[2] = 0;
	if (bUseDropship)
	{
		SpawnDropship(Squad);
	} else
	{
		for (auto elem : Squad)
		{
			for (int i = 0; i < elem.Value; ++i)
			{
				FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
				FRotator Rot = FRotator(0,0,0);
				ACharacterBase* Char = SpawnUnit(elem.Key, Loc, Rot);
				if (Char)
				{
					Spawned.Add(Char);
				}
			}
		}
		for (auto elem : SquadVehicles)
		{
			for (int i = 0; i < elem.Value; ++i)
			{
				FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
				FRotator Rot = FRotator(0,0,0);
				GetWorld()->SpawnActor(elem.Key, &Loc, &Rot);
			}
		}
	}
	OnAvailable.Broadcast(this);
}

void AAISpawner::SpawnSquad(TMap<TSubclassOf<ACharacterBase>, int> SquadToSpawn, TMap<TSubclassOf<AVehicleBase>, int> Vehicles)
{
	SpawnedChars.Empty();
	TArray<ACharacterBase*> Spawned;
	
	FVector BoxExtents = Box->GetScaledBoxExtent();
	BoxExtents[2] = 0;
	if (bUseDropship)
	{
		SpawnDropship(SquadToSpawn);
	} else
	{
		for (auto elem : SquadToSpawn)
		{
			for (int i = 0; i < elem.Value; ++i)
			{
				FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
				FRotator Rot = FRotator(0,0,0);
				ACharacterBase* Char = SpawnUnit(elem.Key, Loc, Rot);
				if (Char)
				{
					Spawned.Add(Char);
				}
			}
		}
		for (auto elem : Vehicles)
		{
			for (int i = 0; i < elem.Value; ++i)
			{
				FVector Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), BoxExtents);
				FRotator Rot = FRotator(0,0,0);
				FActorSpawnParameters ActorSpawnParameters;
				ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				GetWorld()->SpawnActor(elem.Key, &Loc, &Rot, ActorSpawnParameters);
			}
		}
	}
	OnAvailable.Broadcast(this);
}

ACharacterBase* AAISpawner::SpawnUnit(TSubclassOf<ACharacterBase> Unit, FVector SpawnLoc, FRotator SpawnRot)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ACharacterBase* Char = GetWorld()->SpawnActor<ACharacterBase>(Unit, SpawnLoc, SpawnRot);
	if (!Char) return nullptr;
	Char->GetHealthComponent()->OnHealthUpdate.AddDynamic(this, &AAISpawner::OnUnitKilled);
	if (AFirefightGameMode* FirefightGameMode = Cast<AFirefightGameMode>(GetWorld()->GetAuthGameMode()))
	{
		FirefightGameMode->ManageCharacter(Char);
	}
	
	SpawnedChars.Add(Char);
	if (SmartObj)
	{
		//Maybe reenable in the future? Unsure if needed
		//Char->TeamId = DefaultTeam;
		Char->SmartObject = SmartObj;
	}
	// if (UseDropPod)
	// {
	// 	RequestDropPod(Char, SpawnLoc);
	// }

	return Char;
}

