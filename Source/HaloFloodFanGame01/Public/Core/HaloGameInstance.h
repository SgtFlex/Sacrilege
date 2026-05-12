// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HaloGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UHaloGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void Supercombine(AActor* Actor);

	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION(BlueprintImplementableEvent)
	void AddLoadScreen();

	UFUNCTION(BlueprintImplementableEvent)
	void RemoveLoadScreen();
};
