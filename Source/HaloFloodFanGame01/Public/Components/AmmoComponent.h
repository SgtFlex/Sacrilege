// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmmoComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HALOFLOODFANGAME01_API UAmmoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAmmoComponent();
	
	//The maximum amount of bullets the weapon can have in reserve.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 AmmoTotal = 10;

	//The amount of bullets this weapon spawns with in reserve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 CurrentAmmo = AmmoTotal;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetAmmo();
	
	UFUNCTION(BlueprintCallable)
	void SetAmmo(int32 NewAmmo);

	UFUNCTION(BlueprintCallable)
	void ResetAmmo();

	UFUNCTION(BlueprintCallable)
	int32 DecrementAmmo();

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool HasAnyAmmo() const;
};
