// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponInfoHUD.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UWeaponInfoHUD : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
public:
	UFUNCTION(BlueprintNativeEvent)
	void ConstructAmmoGrid(AGunBase* Gun);

	UFUNCTION(BlueprintNativeEvent)
	void SetAmmoReserveCounter(int32 AmmoReserve);

	UFUNCTION()
	void UpdateHUDMagazineElements(); //Maybe clean this up later
	
	UFUNCTION(BlueprintNativeEvent)
	void SetMagazineReserveCounter(int32 MagazineCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetAmmoGridBullets(int32 CurMagazine, int32 MaxMagazine);

	UFUNCTION()
	void UpdateHUDWeaponData(AGunBase* EquippedGun, AGunBase* HolsteredGun);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidget* WeaponHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* AmmoGrid;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AmmoReserveCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MagazineCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MagazineTotal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> AmmoGridChildClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* EquippedGunWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* HolsteredGunWidget;

	UPROPERTY()
	TArray<UUserWidget*> BulletIcons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FLinearColor HUDColor = FColor(255, 150, 50, 255);
	
protected:
	UPROPERTY()
	class ACharacterBase* PlayerCharacter;

};
