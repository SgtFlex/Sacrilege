// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HaloFloodFanGame01/ProjectileBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "HaloWorldSubsystem.generated.h"

class AProjectileBase;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FNeedleStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* Actor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TArray<AProjectileBase*> Projectiles = TArray<AProjectileBase*>();

	bool operator==(const FNeedleStruct& Other) const
	{
		return Actor == Other.Actor;
	}	
};

UCLASS(Blueprintable)
class HALOFLOODFANGAME01_API UHaloWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	FNeedleStruct AddNeedleToActor(AActor* Actor, AProjectileBase* Needle);

	UFUNCTION(BlueprintCallable)
	void GetEmbeddedNeedles(AActor* Actor, TArray<AProjectileBase*>& Needles);

	UFUNCTION(BlueprintCallable)
	void RemoveNeedleFromActor(AActor* Actor, AProjectileBase* Needle);

	UFUNCTION(BlueprintCallable)
	void RemoveAllNeedlesFromActor(AActor* Actor);



public:
	UPROPERTY(BlueprintReadOnly)
	TArray<FNeedleStruct> NeedledActorArray;
};
