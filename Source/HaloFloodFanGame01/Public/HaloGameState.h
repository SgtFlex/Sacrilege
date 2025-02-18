// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HaloGameState.generated.h"





UCLASS()
class HALOFLOODFANGAME01_API AHaloGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GamestateHUD;
};
