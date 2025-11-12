// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrenadeInventoryHUD.generated.h"

struct FGrenadeStruct;
class AGrenadeBase;
class UGrenadeWidget;
class UUniformGridPanel;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeArrayUpdate, int, length);

UCLASS()
class HALOFLOODFANGAME01_API UGrenadeInventoryHUD : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
protected:
	UFUNCTION() 
	void UpdateSelectedGrenadeType(TSubclassOf<AGrenadeBase> GrenadeClass, int GrenadeIndex);

	UFUNCTION()
	void UpdateGrenadeInventory(TArray<FGrenadeStruct>& GrenadeInventory);

	UFUNCTION()
	void CreateGrenadeWidgets();

	UFUNCTION()
	void UpdateGrenadeWidgets();

	UFUNCTION()
	void DestroyGrenadeWidgets();

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel* FragHUD;
	
	UPROPERTY()
	TArray<UGrenadeWidget*> GrenadeWidgets;

	//UPROPERTY()
	//TMap<TSubclassOf<AGrenadeBase>, UGrenadeWidget*> GrenadeWidgetMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GrenadeWidgetClass;

	UPROPERTY()
	TSubclassOf<AGrenadeBase> SelectedGrenadeType;

	int SelectedGrenadeIndex = 0;
	
	UPROPERTY()
	class ACharacterBase* PlayerCharacter;

	UPROPERTY()
	TArray<FGrenadeStruct> OldGrenadeInventory;

public:
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere)
	FOnGrenadeArrayUpdate OnGrenadeArrayUpdate;
};
