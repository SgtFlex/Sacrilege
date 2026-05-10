// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PickupInterface.h"
#include "GameFramework/Actor.h"
#include "EquipmentBase.generated.h"

class UPickupComponent;

UCLASS(Abstract)
class HALOFLOODFANGAME01_API AEquipmentBase : public AActor, public IPickupInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEquipmentBase();

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const { TagContainer = GameplayTags; return; }

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Pickup(ACharacterBase* Character) override;

	UFUNCTION(BlueprintCallable, BlueprintPUre)
	UStaticMeshComponent* GetMesh();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Charges = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UPickupComponent* PickupComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* Mesh;

	
};
