// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

UCLASS()
class HALOFLOODFANGAME01_API AGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	virtual void PrimaryAttack();

	UFUNCTION(BlueprintNativeEvent)
	virtual void SecondaryAttack();

	UFUNCTION(BlueprintNativeEvent)
	virtual void TertiaryAttack();

	UFUNCTION(BlueprintNativeEvent)
	virtual void Fire();

	UFUNCTION(BlueprintNativeEvent)
	virtual void Reload();

public:
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> PrimProj;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> SecProj;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> TertProj;

};
