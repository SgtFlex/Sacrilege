// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrenadeWidget.generated.h"

class AGrenadeBase;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UGrenadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetGrenadeClass(const TSubclassOf<AGrenadeBase>& NewGrenadeClass);

	void SetGrenadeCount(int NewCount) const;

	void SetIsSelected(bool NewIsSelected) const;
public:

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* SelectionBorder;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* GrenadeImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* GrenadeCounter;
	
	UPROPERTY()
	TSubclassOf<AGrenadeBase> GrenadeClass;
};
