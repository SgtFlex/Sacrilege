// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldCleanupManager.generated.h"

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
class HALOFLOODFANGAME01_API UWorldCleanupManager : public UWorldSubsystem
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
