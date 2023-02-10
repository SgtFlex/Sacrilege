// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloHUDWidget.h"
#include "Styling/SlateTypes.h"
#include "Components/ProgressBar.h"

void UHaloHUDWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthBar) {
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void UHaloHUDWidget::SetShields_Implementation(float CurrentShields, float MaxShields)
{
	if (ShieldBar) {
		ShieldBar->SetPercent(CurrentShields / MaxShields);
	}
}
