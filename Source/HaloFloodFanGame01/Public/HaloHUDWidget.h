// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HaloHUDWidget.generated.h"


/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UHaloHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
public:
	UFUNCTION(BlueprintNativeEvent)
	void SetHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintNativeEvent)
	void SetShields(float CurrentShields, float MaxShields);

	UFUNCTION(BlueprintNativeEvent)
	void SetCanInteract(bool CanInteract);

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

	virtual bool Initialize() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UProgressBar* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UProgressBar* ShieldBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* InteractText;

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
	class UTextBlock* AmmoReserveCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> AmmoGridChildClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Compass;
};
