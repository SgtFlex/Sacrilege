// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AISpawner.generated.h"

class AVehicleBase;
class UHealthComponent;
class ASmartObject;
class UBoxComponent;
class ACharacterBase;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvailable, AAISpawner*, Spawner);

UCLASS()
class HALOFLOODFANGAME01_API AAISpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAISpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnUnitKilled(UHealthComponent* HealthComponent);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void TriggerSpawn();

	UFUNCTION(BlueprintCallable)
	void SpawnSquad(TMap<TSubclassOf<ACharacterBase>, int> SquadToSpawn, TMap<TSubclassOf<AVehicleBase>, int> Vehicles);

	UFUNCTION(BlueprintCallable)
	ACharacterBase* SpawnUnit(TSubclassOf<ACharacterBase> Unit, FVector SpawnLoc, FRotator SpawnRot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	AVehicleBase* SpawnDropPods(const TMap<TSubclassOf<ACharacterBase>, int>& SquadToSpawn, FVector DropLocation);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	AVehicleBase* SpawnDropship(const TMap<TSubclassOf<ACharacterBase>, int>& SquadToSpawn);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ACharacterBase>, int> Squad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<ACharacterBase>, int> SquadVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObj;

	UPROPERTY()
	TArray<ACharacterBase*> SpawnedChars;

	UPROPERTY()
	FOnAvailable OnAvailable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUseDropship = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUseDropPod = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCanSpawnVehicles = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 DefaultTeam = 2;

};
