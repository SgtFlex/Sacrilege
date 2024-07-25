// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NavMovementComponent.h"
#include "HoverMovementComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class HALOFLOODFANGAME01_API UHoverMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	virtual void ThrustToTargetSpeed();

	UFUNCTION(BlueprintCallable)
	virtual void TorqueToTargetRotation();
public:
	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector TargetForceLocal;
	FVector CurrentVelocity;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere)
	float ThrustForce = 1000000.0;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FRotator TargetRotation;
	FRotator CurrentRotation;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector TargetForward;
	FVector CurrentForward;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueForce = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueDamping = 0.5f;
	
};
