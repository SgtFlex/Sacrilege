// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeWidget.h"

#include "GrenadeBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGrenadeWidget::SetGrenadeClass(const TSubclassOf<AGrenadeBase>& NewGrenadeClass)
{
	GrenadeClass = NewGrenadeClass;
	GrenadeImage->SetBrushFromTexture(GrenadeClass.GetDefaultObject()->GrenadeIcon);
}

void UGrenadeWidget::SetGrenadeCount(int NewCount) const
{
	GrenadeCounter->SetText(FText::AsNumber(NewCount));
}

void UGrenadeWidget::SetIsSelected(bool NewIsSelected) const
{
	if (NewIsSelected)
	{
		SelectionBorder->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}
