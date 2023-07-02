// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "HaloHUDWidget.generated.h"


class UListView;
class UTextBlock;
class UHealthComponent;
class AHaloFloodFanGame01Character;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UHaloHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintNativeEvent)
	void SetHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintNativeEvent)
	void SetShields(float CurrentShields, float MaxShields);

	UFUNCTION(BlueprintNativeEvent)
	void SetCanInteract(bool CanInteract);

	UFUNCTION()
	void SetInteractInfo(FText InfoText, UTexture2D* Icon = nullptr);

	UFUNCTION(BlueprintNativeEvent)
	void SetFragCounter(int32 NewFragCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetPlasmaCounter(int32 NewPlasmaCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetSpikeCounter(int32 NewSpikeCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetIncenCounter(int32 NewIncenCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetCompassDirection(float Yaw);

	

	UFUNCTION(BlueprintNativeEvent)
	void ConstructAmmoGrid(AGunBase* Gun);

	UFUNCTION(BlueprintNativeEvent)
	void SetAmmoReserveCounter(int32 AmmoReserve);

	UFUNCTION()
	void UpdateHUDMagazineElements(); //Maybe clean this up later

	UFUNCTION()
	void OnScoreUpdated(int NewScore);

	UFUNCTION(BlueprintNativeEvent)
	void SetMagazineReserveCounter(int32 MagazineCount);

	UFUNCTION(BlueprintNativeEvent)
	void SetAmmoGridBullets(int32 CurMagazine, int32 MaxMagazine);
	
	void SetCrosshairType(int type);

	void SetCrosshairTexture(UTexture2D* NewTexture);

	UFUNCTION(BlueprintCallable)
	void SetFragHUDEnabled(bool bDisplay);

	UFUNCTION(BlueprintCallable)
	void SetWeaponHUDEnabled(bool bDisplay);

	UFUNCTION(BlueprintCallable)
	void OnHealthUpdated(UHealthComponent* HealthComp);

	UFUNCTION()
	void UpdateHUDWeaponData(AGunBase* EquippedGun, AGunBase* HolsteredGun);

	UFUNCTION()
	void UpdateSetAndWaveCount(int Set, int Wave);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PushTextNotification(const FText& Text);
	
	virtual bool Initialize() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* WaveCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SetCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidget* WeaponHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidget* FragHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidget* HealthHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidget* CompassHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UListView* TextNotificationList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Crosshair;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HealthNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* ShieldBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ShieldNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* InteractBoxWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* InteractActionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* AmmoGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FragCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlasmaCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SpikeCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* IncenCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScoreCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AmmoReserveCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MagazineCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> AmmoGridChildClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* EquippedGunWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* HolsteredGunWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Compass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CompassNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* InteractIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CompassDirection;

	TArray<UUserWidget*> BulletIcons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Colors")
	FLinearColor EnemyColor = FColor(255, 25, 25, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Colors")
	FLinearColor AllyColor = FColor(25, 255, 25, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Colors")
	FLinearColor InteractableColor = FColor(25, 25, 255, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FLinearColor HUDColor = FColor(255, 150, 50, 255);

	UPROPERTY()
	class AHaloFloodFanGame01Character* PlayerCharacter;
};