// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FLinearColor HUDColor = FColor(255, 150, 50, 255);
};