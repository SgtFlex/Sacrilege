// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Perception/AISightTargetInterface.h"
#include "HaloSpectatorPawn.generated.h"

struct FInputActionValue;
/**
 * 
 */
//test comment
UCLASS()
class HALOFLOODFANGAME01_API AHaloSpectatorPawn : public ASpectatorPawn, public IAISightTargetInterface
{
	GENERATED_BODY()
	
	virtual void LookUpAtRate(float Rate) override;

	virtual void TurnAtRate(float Rate) override;
	
	/** Called for looking input */
	
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const override;

public:
protected:
	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
};
