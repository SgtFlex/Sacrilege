// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HaloGameState.generated.h"

class ACharacterBase;
/**
 * 
 */
USTRUCT()
struct FManagedActorStruct
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AActor> ActorClass;

	uint8 ActorLimit;

	UPROPERTY()
	TArray<AActor*> ManagedActors;

	bool operator==(const FManagedActorStruct& other) const
	{
		return other.ActorClass == ActorClass;
	}
};

UCLASS()
class HALOFLOODFANGAME01_API AHaloGameState : public AGameState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageRagdoll(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageDecal(UDecalComponent* Decal);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageWeapon(AActor* Weapon);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingWeapon(AActor* Weapon);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageActor(AActor* Actor);

public:
	//UPROPERTY()
	//TMap<AActor*, uint8> ManagedActors;
	
	UPROPERTY()
	TArray<AActor*> Ragdolls;

	UPROPERTY()
	TArray<UDecalComponent*> Decals;

	UPROPERTY()
	TArray<AActor*> Weapons;

	UPROPERTY()
	TArray<FManagedActorStruct> ManagedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxRagdolls = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxWeapons = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxDecals = 20;
};
