// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BasePawn.h"
#include "BaseGroundCharacter.generated.h"

class UCharacterMovementComponent;
class UMovementComponent;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API ABaseGroundCharacter : public ABasePawn
{
	GENERATED_BODY()

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UCharacterMovementComponent* CharMoveComp;
public:
	ABaseGroundCharacter();
};
