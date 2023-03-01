// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloPlayerHUD.h"

#include "HaloFloodFanGame01Character.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

AHaloPlayerHUD::AHaloPlayerHUD()
{

}



void AHaloPlayerHUD::SetHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthBar) {
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void AHaloPlayerHUD::SetShields(float CurrentShields, float MaxShields)
{
	if (ShieldBar) {
		ShieldBar->SetPercent(CurrentShields / MaxShields);
	}
}

