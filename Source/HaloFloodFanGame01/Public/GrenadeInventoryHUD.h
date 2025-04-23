// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrenadeInventoryHUD.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UGrenadeInventoryHUD : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION() 
	void UpdateSelectedGrenadeType(TSubclassOf<AGrenadeBase> GrenadeClass);

	UFUNCTION()
	void UpdateGrenadeInventory(TArray<FGrenadeStruct> GrenadeInventory);

	UFUNCTION()
	void CreateGrenadeWidgets();

	UFUNCTION()
	void UpdateGrenadeWidgets();

	UFUNCTION()
	void DestroyGrenadeWidgets();

public:
	UPROPERTY()
	TArray<UGrenadeWidget*> GrenadeWidgets;

	UPROPERTY()
	TMap<TSubclassOf<AGrenadeBase>, UGrenadeWidget*> GrenadeWidgetMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GrenadeWidgetClass;

	UPROPERTY()
	TSubclassOf<AGrenadeBase> SelectedGrenadeType;

protected:
	UPROPERTY()
	class ACharacterBase* PlayerCharacter;

	UPROPERTY()
	TArray<FGrenadeStruct> OldGrenadeInventory;
};
