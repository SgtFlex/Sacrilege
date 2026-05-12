// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponInfoHUD.h"

#include "GunBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Core/CharacterBase.h"

void UWeaponInfoHUD::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacter = Cast<ACharacterBase>(GetOwningPlayerPawn());
	check(PlayerCharacter);
	PlayerCharacter->WeaponsUpdated.AddDynamic(this, &UWeaponInfoHUD::UpdateHUDWeaponData);
	UpdateHUDWeaponData(PlayerCharacter->EquippedWeapon, PlayerCharacter->HolsteredWeapon);
}

void UWeaponInfoHUD::ConstructAmmoGrid_Implementation(const AGunBase* Gun)
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

void UWeaponInfoHUD::SetAmmoReserveCounter_Implementation(int32 AmmoReserve)
{
	AmmoReserveCounter->SetText(FText::AsNumber(AmmoReserve));
}

void UWeaponInfoHUD::SetAmmoGridBullets_Implementation(int32 CurMagazine, int32 MaxMagazine)
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

void UWeaponInfoHUD::SetMagazineReserveCounter_Implementation(int32 MagazineCount)
{
	MagazineCounter->SetText(FText::AsNumber(MagazineCount));
}


void UWeaponInfoHUD::UpdateHUDMagazineElements()
{
	if (!PlayerCharacter || !PlayerCharacter->EquippedWeapon) return;
	SetMagazineReserveCounter(EquippedGun->CurMagazine);
	SetAmmoReserveCounter(EquippedGun->CurReserve);
	SetAmmoGridBullets(EquippedGun->CurMagazine, EquippedGun->MaxMagazine);
}

//@TODO We can potentially change this later to not use casting
void UWeaponInfoHUD::UpdateHUDWeaponData(AWeaponBase* EquippedWeapon, AWeaponBase* HolsteredWeapon)
{
	EquippedGun = Cast<AGunBase>(EquippedWeapon);
	if (EquippedGun)
	{
		//SetCrosshairTexture(EquippedGun->CrosshairTexture);
		MagazineCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoGrid->SetVisibility(ESlateVisibility::Visible);
		SetAmmoReserveCounter(EquippedGun->CurReserve);
		EquippedGunWidget->SetBrushFromTexture(EquippedGun->WeaponIcon);
		ConstructAmmoGrid(EquippedGun);
		UpdateHUDMagazineElements();
		MagazineTotal->SetText(FText::AsNumber(EquippedGun->MaxMagazine));
		// EquippedGun->OnFire.AddUniqueDynamic(this, &UWeaponInfoHUD::UpdateHUDMagazineElements);
		// EquippedGun->OnReload.AddUniqueDynamic(this, &UWeaponInfoHUD::UpdateHUDMagazineElements);
		EquippedGun->OnAmmoUpdated.AddUniqueDynamic(this, &UWeaponInfoHUD::UpdateHUDMagazineElements);
	} else
	{
		MagazineCounter->SetVisibility(ESlateVisibility::Hidden);
		AmmoGrid->SetVisibility(ESlateVisibility::Hidden);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Hidden);
	}
	if (HolsteredWeapon)
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Visible);
		HolsteredGunWidget->SetBrushFromTexture(HolsteredWeapon->WeaponIcon);
	} else
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}