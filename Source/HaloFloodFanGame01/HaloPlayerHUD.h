// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HaloPlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API AHaloPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	void SetHealth(float CurrentHealth, float MaxHealth);

	void SetShields(float CurrentShields, float MaxShields);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
		class UProgressBar* HealthBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
		class UProgressBar* ShieldBar;


	
};
