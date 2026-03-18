// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TipSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UTipSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "TipSubsystem")
	void TryDisplayTip(FText Text);
public:
	UPROPERTY(EditAnywhere)
	TMap<TSubclassOf<UUserWidget>, FTimerHandle> Tips;
};
