// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldCleanupManager.generated.h"

/**
 * 
 */

//TODO Perhaps utilize this struct to dynamically add more types of managed actors. Maybe use Tags or Strings or something instead of a class.
USTRUCT()
struct FManagedActorStruct
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name = "";

	UPROPERTY()
	uint8 ActorLimit = 100;

	UPROPERTY()
	TArray<AActor*> ManagedActors;
};

UCLASS()
class HALOFLOODFANGAME01_API UWorldCleanupManager : public UWorldSubsystem
{
	GENERATED_BODY()

	UWorldCleanupManager();
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageCorpse(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingCorpse(AActor* Ragdoll);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageDecal(UDecalComponent* Decal);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingDecal(UDecalComponent* Decal);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageWeapon(AActor* Weapon);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingWeapon(AActor* Weapon);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageGrenade(AActor* Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingGrenade(AActor* Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ManageActor(const FString& Category, AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopManagingActor(const FString& Category, AActor* Actor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddCleanupCategory(const FString& Category, uint8 ActorLimit);
	
public:
	UPROPERTY()
	TArray<AActor*> Ragdolls;

	UPROPERTY()
	TArray<UDecalComponent*> Decals;

	UPROPERTY()
	TArray<AActor*> Weapons;

	UPROPERTY()
	TArray<AActor*> Grenades;

	UPROPERTY()
	TArray<FManagedActorStruct> CleanupStructs;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxCorpses = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxWeapons = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxDecals = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxGrenades = 25;
	
};
