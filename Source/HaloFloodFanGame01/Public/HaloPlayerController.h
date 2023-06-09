// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "HaloPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API AHaloPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	AHaloPlayerController();

private:
	FGenericTeamId TeamId;
	FGenericTeamId GetGenericTeamId() const;
};
