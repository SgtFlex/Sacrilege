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
#include "Components/VerticalBox.h"
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

	FHitResult PlayerAim = Character->GetPlayerAim();
	if (Cast<IDamageableInterface>(PlayerAim.GetActor()) && PlayerAim.Distance <= 2000)
	{
		SetCrosshairType(4);
	} else if (Cast<IInteractableInterface>(PlayerAim.GetActor()) && PlayerAim.Distance <= 500)
	{
		SetCrosshairType(2);
	} else
	{
		SetCrosshairType(1);
	}

	if (Character && Character->EquippedWep && Character->EquippedWep->CrosshairTex) Crosshair->SetBrushFromTexture(Character->EquippedWep->CrosshairTex);
	SetFragCounter(Character->FragCount);
	IInteractableInterface* IntActor = Cast<IInteractableInterface>(PlayerAim.GetActor());
	if (PlayerAim.bBlockingHit && IsValid(PlayerAim.GetActor()) && IntActor)
	{
		FText IntText;
		UTexture2D* IntIcon;
		IntActor->Execute_GetInteractInfo(PlayerAim.GetActor(), IntText, IntIcon);
		SetInteractInfo(IntText, IntIcon);
		SetCanInteract(true);
	}
	else 
		SetCanInteract(false);
}

void UHaloHUDWidget::SetInteractInfo(FText InfoText, UTexture2D* Icon)
{
	TestText = InfoText;
	
	
	if (Icon)
	{
		InteractIcon->SetBrushFromTexture(Icon);
		InteractIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractIcon->SetVisibility(ESlateVisibility::Hidden);
	}
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

void UHaloHUDWidget::SetAmmoGridBullets_Implementation(int32 CurMagazine, int32 MaxMagazine)
{
	int i = 0;
	for (auto BulletIcon : BulletIcons)
	{
		if (i < CurMagazine)
			BulletIcon->SetColorAndOpacity(HUDColor-FLinearColor(0,0,0,.5));
		else
			BulletIcon->SetColorAndOpacity(FLinearColor(0,0,0, .25));
		i++;
	}
}

void UHaloHUDWidget::SetMagazineReserveCounter_Implementation(int32 MagazineCount)
{
	MagazineCounter->SetText(FText::AsNumber(MagazineCount));
	InteractActionWidget->SetText(TestText);
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

void UHaloHUDWidget::UpdateHUDWeaponData(AGunBase* EquippedGun, AGunBase* HolsteredGun)
{
	if (EquippedGun)
	{
		MagazineCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoGrid->SetVisibility(ESlateVisibility::Visible);
		SetMagazineReserveCounter(EquippedGun->CurMagazine);
		SetAmmoReserveCounter(EquippedGun->CurReserve);
		EquippedGunWidget->SetBrushFromTexture(EquippedGun->WeaponIcon);
		ConstructAmmoGrid(EquippedGun);
		SetAmmoGridBullets(EquippedGun->CurMagazine, EquippedGun->MaxReserve);
	} else
	{
		MagazineCounter->SetVisibility(ESlateVisibility::Hidden);
		AmmoGrid->SetVisibility(ESlateVisibility::Hidden);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Hidden);
	}
	if (HolsteredGun)
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Visible);
		HolsteredGunWidget->SetBrushFromTexture(HolsteredGun->WeaponIcon);
	} else
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Hidden);
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
	if (CanInteract)
		InteractBoxWidget->SetVisibility(ESlateVisibility::Visible);
	else
		InteractBoxWidget->SetVisibility(ESlateVisibility::Hidden);
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


