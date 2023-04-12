// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

class UHaloHUDWidget;
class ABaseCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFire);

UCLASS()
class HALOFLOODFANGAME01_API AGunBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintAssignable)
	FOnFire OnFire;
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Pickup(ABaseCharacter* Char);

	void Drop();

	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	UFUNCTION(BlueprintNativeEvent)
	void Reload();

	UFUNCTION(BlueprintNativeEvent)
	void PullTrigger();

	UFUNCTION(BlueprintNativeEvent)
	void ReleaseTrigger();
	
	virtual void OnInteract_Implementation(AHaloFloodFanGame01Character* Character) override;

	virtual void GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon) override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> CamShake;
	
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	TSubclassOf<AActor> Projectile;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float Damage = 15;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float Force = 1000;
	
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float FireRate = 500;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	int32 Multishot = 1;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	int32 BurstAmount = 0; //0 means full auto, 1 means semi auto, 2+ is burst fire of said amount

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float BurstRetriggerDelay = 0.15f; //seconds required before bursting again

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float ReloadSpeed = 3;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float HorizontalRecoil = 1;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float VerticalRecoil = 1;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float Accuracy = 100;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float Range = 5000;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	int32 MaxMagazine = 32;

	int32 CurMagazine;
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	int32 MaxReserve = 160;

	int32 CurReserve;

	UPROPERTY(EditAnywhere, meta = (Category="FiringSound"))
	USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, meta = (Category="HUD"))
	UTexture2D* CrosshairTex;

	UPROPERTY(EditAnywhere, meta = (Category="HUD"))
	UTexture2D* WeaponIcon;

	UPROPERTY(EditAnywhere, meta = (Category="FiringSound"))
	USoundAttenuation* FiringAttenuation;

	int32 BulletsFired;

	FText InteractText = FText::FromString("Pickup");

	UPROPERTY()
	UTexture2D* InteractIcon = WeaponIcon;

	FTimerHandle FireHandle;

	FTimerHandle BurstRetriggerHandle;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* MuzzlePFX;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="HUD"))
	TSubclassOf<UUserWidget> BulletWidget;

private:
	UPROPERTY()
	UHaloHUDWidget* PlayerHUD;
};
