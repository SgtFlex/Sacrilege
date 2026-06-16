// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StabilizerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HALOFLOODFANGAME01_API UStabilizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStabilizerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TurnToTargetOrientation();

	UFUNCTION(BlueprintCallable)
	void SetTargetOrientation(FQuat NewOrientation);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetTargetOrientation(FQuat& OutOrientation);

public:
	UPROPERTY(EditAnywhere)
	float Damping = 0.5f;

	UPROPERTY(EditAnywhere)
	float TorqueForce = 15.0f;

	UPROPERTY(EditAnywhere)
	float OverallStrength = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bStabilizePitch = true;

	UPROPERTY(EditAnywhere)
	bool bStabilizeYaw = true;
	
	UPROPERTY(EditAnywhere)
	bool bStabilizeRoll = true;
	
	UPROPERTY(BlueprintReadOnly)
	FQuat CurrentOrientation;

	UPROPERTY(BlueprintReadOnly)
	FQuat TargetOrientation;
	
private:

	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;
};
