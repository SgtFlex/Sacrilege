// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	ABaseAIController();

public:
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};
