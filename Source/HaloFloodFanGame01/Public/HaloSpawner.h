// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HaloSpawner.generated.h"

class ASmartObject;
class UBoxComponent;
class ABaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawned);

UCLASS()
class HALOFLOODFANGAME01_API AHaloSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHaloSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TArray<ABaseCharacter*> SpawnSquad(TMap<TSubclassOf<ABaseCharacter>, int> SquadToSpawn);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ABaseCharacter>, int> Squad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObj;

	UPROPERTY()
	TArray<ABaseCharacter*> SpawnedChars;

};
