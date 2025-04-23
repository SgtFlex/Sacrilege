// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"

#include "GrenadeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/GridPanel.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "GunBase.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridSlot.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "GrenadeBase.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacter = Cast<ACharacterBase>(GetOwningPlayerPawn());
	check(PlayerCharacter);
	PlayerCharacter->WeaponsUpdated.AddDynamic(this, &UPlayerHUD::UpdateHUDWeaponData);
	//PlayerCharacter->OnInteractableChanged.AddDynamic(this, &UPlayerHUD::UpdateInteractable);
	//PlayerCharacter->GetHealthComponent()->OnHealthUpdate.AddDynamic(this, &UPlayerHUD::OnHealthUpdated);
	PlayerCharacter->OnGrenadeInventoryUpdated.AddDynamic(this, &UPlayerHUD::UpdateGrenadeInventory);
	PlayerCharacter->OnGrenadeTypeSwitched.AddDynamic(this, &UPlayerHUD::UPlayerHUD::UpdateSelectedGrenadeType);
	UpdateGrenadeInventory(PlayerCharacter->GetGrenadeInventory());
	UpdateSelectedGrenadeType(PlayerCharacter->GetSelectedGrenadeType());
	UpdateHUDWeaponData(PlayerCharacter->EquippedWeapon, PlayerCharacter->HolsteredWeapon);
}

void UPlayerHUD::UpdateSelectedGrenadeType(TSubclassOf<AGrenadeBase> GrenadeClass)
{
	if (GrenadeWidgets.IsEmpty()) return;
	if (SelectedGrenadeType)
	{
		GrenadeWidgetMap[SelectedGrenadeType]->SetIsSelected(false);
	}
	SelectedGrenadeType = GrenadeClass;
	if (!GrenadeWidgetMap.IsEmpty() && GrenadeWidgetMap.Contains(PlayerCharacter->GetSelectedGrenadeType()))
	{
		GrenadeWidgetMap[GrenadeClass]->SetIsSelected(true);
	}
}

void UPlayerHUD::UpdateGrenadeInventory(TArray<FGrenadeStruct> GrenadeInventory)
{
	//If old grenade inventory size doesn't match new inventory size, then reconstruct the widget array (NOT MAP). BUT FIRST CHECK IF SIZE IS THE SAME.
	if (GrenadeInventory.Num() == OldGrenadeInventory.Num() && GrenadeInventory.Num() == GrenadeWidgets.Num())
	{
		//Update the existing widgets
		UpdateGrenadeWidgets();
	} else
	{
		//Reconstruct the widgets
		DestroyGrenadeWidgets();
		
		CreateGrenadeWidgets();
	}
	OldGrenadeInventory = GrenadeInventory;
}

void UPlayerHUD::CreateGrenadeWidgets()
{
	const TArray<FGrenadeStruct> GrenadeInventory = PlayerCharacter->GetGrenadeInventory();
	for (int i = 0; i < GrenadeInventory.Num(); i++)
	{
		UGrenadeWidget* GrenadeWidget = CreateWidget<UGrenadeWidget>(FragHUD, GrenadeWidgetClass);
		UUniformGridSlot* GridSlot = FragHUD->AddChildToUniformGrid(GrenadeWidget);
		GridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		GridSlot->SetColumn(i);

		GrenadeWidget->SetGrenadeClass(GrenadeInventory[i].GrenadeClass);
		GrenadeWidget->SetGrenadeCount(GrenadeInventory[i].GrenadeAmount);
		GrenadeWidgets.Add(GrenadeWidget);
		GrenadeWidgetMap.Add(GrenadeInventory[i].GrenadeClass, GrenadeWidget);
	}
	SelectedGrenadeType = nullptr;
	UpdateSelectedGrenadeType(PlayerCharacter->GetSelectedGrenadeType());
}

void UPlayerHUD::UpdateGrenadeWidgets()
{
	const TArray<FGrenadeStruct> GrenadeInventory = PlayerCharacter->GetGrenadeInventory();
	for (int i = 0; i < GrenadeInventory.Num(); i++)
	{
		if (GrenadeInventory[i].GrenadeClass == OldGrenadeInventory[i].GrenadeClass)
		{
			GrenadeWidgets[i]->SetGrenadeCount(GrenadeInventory[i].GrenadeAmount);
		} else if (GrenadeInventory[i].GrenadeClass != OldGrenadeInventory[i].GrenadeClass)
		{
			GrenadeWidgets[i]->SetGrenadeClass(GrenadeInventory[i].GrenadeClass);
			GrenadeWidgets[i]->SetGrenadeCount(GrenadeInventory[i].GrenadeAmount);
			GrenadeWidgetMap[GrenadeInventory[i].GrenadeClass] = GrenadeWidgets[i];
		}
	}
}

void UPlayerHUD::DestroyGrenadeWidgets()
{
	for (int i = 0; i < GrenadeWidgets.Num(); i++)
	{
		GrenadeWidgets[i]->RemoveFromParent();
	}
	GrenadeWidgets.Empty();
	GrenadeWidgetMap.Empty();
}

void UPlayerHUD::ConstructAmmoGrid_Implementation(AGunBase* Gun)
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

void UPlayerHUD::SetAmmoReserveCounter_Implementation(int32 AmmoReserve)
{
	AmmoReserveCounter->SetText(FText::AsNumber(AmmoReserve));
}

void UPlayerHUD::SetAmmoGridBullets_Implementation(int32 CurMagazine, int32 MaxMagazine)
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

void UPlayerHUD::SetMagazineReserveCounter_Implementation(int32 MagazineCount)
{
	MagazineCounter->SetText(FText::AsNumber(MagazineCount));
}


void UPlayerHUD::UpdateHUDMagazineElements()
{
	if (!PlayerCharacter || !PlayerCharacter->EquippedWeapon) return;
	SetMagazineReserveCounter(PlayerCharacter->EquippedWeapon->CurMagazine);
	SetAmmoReserveCounter(PlayerCharacter->EquippedWeapon->CurReserve);
	SetAmmoGridBullets(PlayerCharacter->EquippedWeapon->CurMagazine, PlayerCharacter->EquippedWeapon->MaxMagazine);
}

void UPlayerHUD::UpdateHUDWeaponData(AGunBase* EquippedGun, AGunBase* HolsteredGun)
{
	UE_LOG(LogTemp, Warning, TEXT("Updated HUD Weapon data"));
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
		// EquippedGun->OnFire.AddUniqueDynamic(this, &UPlayerHUD::UpdateHUDMagazineElements);
		// EquippedGun->OnReload.AddUniqueDynamic(this, &UPlayerHUD::UpdateHUDMagazineElements);
		EquippedGun->OnAmmoUpdated.AddUniqueDynamic(this, &UPlayerHUD::UpdateHUDMagazineElements);
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