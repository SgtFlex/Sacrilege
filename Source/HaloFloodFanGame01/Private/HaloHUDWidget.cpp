// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "GunBase.h"
#include "Styling/SlateTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridSlot.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"

void UHaloHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetFragCounter(Character->FragCount);
	SetPlasmaCounter(Character->PlasmaCount);
	SetSpikeCounter(Character->SpikeCount);
	SetIncenCounter(Character->IncenCount);
	
}

void UHaloHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHaloHUDWidget::SetCompassDirection_Implementation(float Yaw)
{
	CompassDirection = (Yaw+180);
	float Offset = 45;
	float x = ((Yaw+Offset)*-10);
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Compass->Slot);
	CanvasSlot->SetPosition(FVector2d(x, 0));
	
}


void UHaloHUDWidget::ConstructAmmoGrid_Implementation(AGunBase* Gun)
{
	if (!Gun->BulletWidget) return;
    AmmoGrid->ClearChildren();
	BulletIcons.Empty();
	int32 Columns = 15;
	
	for (int i = Columns; i > 0; i--)
	{
		if ((Gun->MaxMagazine)%i==0)
		{
			Columns = i;
			break;
		}
	}
	int32 Rows = Gun->MaxMagazine/Columns;
	int CurBullet = 0;
	for (int i = 0; i < Rows; ++i)
	{
		for (int j = 0; j < Columns; ++j)
		{
			UUserWidget* BulletIcon = WidgetTree->ConstructWidget<UUserWidget>(Gun->BulletWidget);
			UUniformGridSlot* UniSlot = AmmoGrid->AddChildToUniformGrid(BulletIcon, i, j);
			UniSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			UniSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			BulletIcons.Add(BulletIcon);
			CurBullet++;
		}
	}
}

void UHaloHUDWidget::SetAmmoReserveCounter_Implementation(int32 AmmoReserve)
{
	AmmoReserveCounter->SetText(FText::AsNumber(AmmoReserve));
}

void UHaloHUDWidget::SetBulletUsed_Implementation(int32 NumSlot, bool bUsed)
{
	if (!BulletIcons[NumSlot]) return;
	BulletIcons[NumSlot]->SetVisibility(ESlateVisibility::Hidden);
}

void UHaloHUDWidget::SetMagazineReserveCounter_Implementation(int32 MagazineCount)
{
	MagazineCounter->SetText(FText::AsNumber(MagazineCount));
}

void UHaloHUDWidget::SetCrosshairType(int type)
{
	switch (type)
	{
	default:
	case 1:
		Crosshair->SetColorAndOpacity(HUDColor);
		break;
	case 2:
		Crosshair->SetColorAndOpacity(InteractableColor);
		break;
	case 3:
		Crosshair->SetColorAndOpacity(AllyColor);
		break;
	case 4:
		Crosshair->SetColorAndOpacity(EnemyColor);
		break;
	}
}

void UHaloHUDWidget::SetAmmoGridWeapon(AGunBase* Weapon)
{
	if (Weapon)
	{
		SetAmmoReserveCounter(Weapon->CurReserve);
		SetMagazineReserveCounter(Weapon->CurMagazine);
		MagazineCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoGrid->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		MagazineCounter->SetVisibility(ESlateVisibility::Hidden);
		AmmoGrid->SetVisibility(ESlateVisibility::Hidden);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UHaloHUDWidget::Initialize()
{
	Super::Initialize();
	
	
	return true;
}

void UHaloHUDWidget::SetHealth_Implementation(float CurrentHealth, float MaxHealth)
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

void UHaloHUDWidget::SetCanInteract_Implementation(bool CanInteract)
{
	if (InteractText)
	{
		if (CanInteract)
			InteractText->SetVisibility(ESlateVisibility::Visible);
		else
			InteractText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHaloHUDWidget::SetFragCounter_Implementation(int32 NewFragCount)
{
	if (FragCounter)
		FragCounter->SetText(FText::AsNumber(NewFragCount));
}

void UHaloHUDWidget::SetPlasmaCounter_Implementation(int32 NewPlasmaCount)
{
	if (PlasmaCounter)
		PlasmaCounter->SetText(FText::AsNumber(NewPlasmaCount));
}

void UHaloHUDWidget::SetSpikeCounter_Implementation(int32 NewSpikeCount)
{
	if (SpikeCounter)
		SpikeCounter->SetText(FText::AsNumber(NewSpikeCount));
}

void UHaloHUDWidget::SetIncenCounter_Implementation(int32 NewIncenCount)
{
	if (IncenCounter)
		IncenCounter->SetText(FText::AsNumber(NewIncenCount));
}


