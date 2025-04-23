// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeInventoryHUD.h"

void UGrenadeInventoryHUD::UpdateSelectedGrenadeType(TSubclassOf<AGrenadeBase> GrenadeClass)
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

void UGrenadeInventoryHUD::UpdateGrenadeInventory(TArray<FGrenadeStruct> GrenadeInventory)
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

void UGrenadeInventoryHUD::CreateGrenadeWidgets()
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

void UGrenadeInventoryHUD::UpdateGrenadeWidgets()
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

void UGrenadeInventoryHUD::DestroyGrenadeWidgets()
{
	for (int i = 0; i < GrenadeWidgets.Num(); i++)
	{
		GrenadeWidgets[i]->RemoveFromParent();
	}
	GrenadeWidgets.Empty();
	GrenadeWidgetMap.Empty();
}