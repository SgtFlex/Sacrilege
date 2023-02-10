// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HaloHUDWidget.generated.h"


/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UHaloHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintNativeEvent)
	void SetShields(float CurrentShields, float MaxShields);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
		class UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UProgressBar* ShieldBar;
};
