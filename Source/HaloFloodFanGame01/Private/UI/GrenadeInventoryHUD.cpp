// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GrenadeInventoryHUD.h"

#include "Core/GrenadeBase.h"
#include "UI/GrenadeWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Core/CharacterBase.h"

void UGrenadeInventoryHUD::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacter = Cast<ACharacterBase>(GetOwningPlayerPawn());
	check(PlayerCharacter);
	PlayerCharacter->OnGrenadeInventoryUpdated.AddDynamic(this, &UGrenadeInventoryHUD::UpdateGrenadeInventory);
	PlayerCharacter->OnGrenadeTypeSwitched.AddDynamic(this, &UGrenadeInventoryHUD::UpdateSelectedGrenadeType);
	UpdateGrenadeInventory(PlayerCharacter->GetGrenadeInventory());
	UpdateSelectedGrenadeType(PlayerCharacter->GetSelectedGrenadeType(), PlayerCharacter->GetGrenadeTypeIndex());
}

void UGrenadeInventoryHUD::UpdateSelectedGrenadeType(TSubclassOf<AGrenadeBase> GrenadeClass, const int GrenadeIndex)
{
	if (GrenadeWidgets.IsEmpty()) return;
	if (SelectedGrenadeIndex < GrenadeWidgets.Num())
	{
		GrenadeWidgets[SelectedGrenadeIndex]->SetIsSelected(false);
	}
	
	SelectedGrenadeIndex = GrenadeIndex;
	GrenadeWidgets[GrenadeIndex]->SetIsSelected(true);
}

void UGrenadeInventoryHUD::UpdateGrenadeInventory(TArray<FGrenadeStruct>& GrenadeInventory)
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
	OnGrenadeArrayUpdate.Broadcast(GrenadeInventory.Num());
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
	}
	SelectedGrenadeType = nullptr;
	UpdateSelectedGrenadeType(PlayerCharacter->GetSelectedGrenadeType(), PlayerCharacter->GetGrenadeTypeIndex());
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
}