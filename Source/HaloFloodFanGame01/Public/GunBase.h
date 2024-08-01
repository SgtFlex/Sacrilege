// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize TraceStart;

	UPROPERTY()
	FVector_NetQuantize TraceEnd;
	
};

class UPlayerHUD;
class ACharacterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReload);

UCLASS()
class HALOFLOODFANGAME01_API AGunBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintAssignable)
	FOnFire OnFire;

	UPROPERTY(BlueprintAssignable)
	FOnReload OnReload;
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnPickup(ACharacterBase* Char);

	void OnEquipped();

	void OnDropped();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	bool CanFire();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SpawnBullet();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayFireFX();

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnTrailFX(FHitResult Hit);
	

	// UFUNCTION(BlueprintCallable, Server, Reliable)
	// void Server_SpawnBullet();
	//
	// UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	// void Multi_SpawnBullet();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void SpawnMuzzleFX();

	// UFUNCTION(Server, Reliable, WithValidation)
	// void Server_Fire();
	//
	// UFUNCTION(NetMulticast, Reliable, WithValidation)
	// void Multi_Fire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ScopeIn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ScopeOut();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartReload();

	UFUNCTION(Server, Reliable)
	void Server_StartReload();

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_StartReload();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void FinishReload();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PullTrigger();

	// UFUNCTION(Server, Reliable)
	// void Server_PullTrigger();
	//
	// UFUNCTION(NetMulticast, Reliable)
	// void Multi_PullTrigger();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReleaseTrigger();
	
	// UFUNCTION(Server, Reliable)
	// void Server_ReleaseTrigger();
	//
	// UFUNCTION(NetMulticast, Reliable)
	// void Multi_ReleaseTrigger();
	
	virtual void OnInteract_Implementation(ACharacterBase* Character) override;

	virtual void GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon) override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> FiringCameraShake;

	//Actor that will be spawned from the barrel of the gun. Otherwise uses hitscan properties below
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	TSubclassOf<AActor> ProjectileClass;

	//Damage of the hitscan applied to the hit actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass"))
	float Damage = 15;

	//Force of the hitscan applied to the hit component if it simulates physics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass"))
	float Force = 1000;

	//Fire rate of the gun in bullets per minute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float FireRate = 500;

	//Amount of hitscan/projectiles that are fired simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 MultiShot = 1;

	//Controls how many bullets are fired when the trigger is pulled. 0 means full auto, 1 means semi auto, 2+ is burst fire of whatever amount is provided
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	int32 BurstAmount = 0; 

	//The minimum delay between bursts if BurstAmount >= 2
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float BurstRetriggerDelay = 0.15f;

	//Amount of seconds required to reload the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float ReloadSpeed = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float DrawSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float HolsterSpeed = 1;

	//How far left/right the gun jumps when firing a bullet
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float HorizontalRecoil = 1;

	//How far up/down the gun jumps when firing a bullet
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float VerticalRecoil = 1;

	//The degrees of horizontal spread of projectiles/hitscan. 90 means anywhere from the left to the front to the right.
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float HorizontalSpread = 0;

	//The degrees of vertical spread of projectiles/hitscan. 90 means anywhere from the down to the front to the up.
	UPROPERTY(EditAnywhere, meta = (Category="Attributes"))
	float VerticalSpread = 0;

	//Range of the hitscan trace
	UPROPERTY(EditAnywhere, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass"))
	float Range = 5000;

	UPROPERTY(EditAnywhere, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass"))
	UCurveFloat* FalloffCurve;

	//The maximum bullets in a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 MaxMagazine = 32;

	//The amount of bullets the weapon spawns with in a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 CurMagazine = MaxMagazine;

	//The maximum amount of bullets the weapon can have in reserve.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 MaxReserve = 160;

	//The amount of bullets this weapon spawns with in reserve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	int32 CurReserve = MaxReserve;
	
	UPROPERTY(EditAnywhere, meta = (Category="SFX"))
	USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, meta = (Category="SFX"))
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, meta = (Category="SFX"))
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, meta = (Category="HUD"))
	UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, meta = (Category="HUD"))
	TSubclassOf<UUserWidget> ScopeWidget;

	UPROPERTY()
	UUserWidget* ScopeOverlay;

	UPROPERTY(BlueprintReadOnly)
	bool ScopeActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ScopeInSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ScopeOutSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ZoomFOV = 20;
	
	UPROPERTY(EditAnywhere, meta = (Category="HUD"))
	UTexture2D* WeaponIcon;

	UPROPERTY()
	UTexture2D* InteractIcon = WeaponIcon;

	int32 BulletsFired;

	FText InteractText = FText::FromString("Pickup");

	FTimerHandle FireHandle;

	FTimerHandle BurstRetriggerHandle;

	FTimerHandle DrawHandle;

	FTimerHandle HolsterHandle;

	bool bReloading = false;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* MuzzlePFX;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailPFX;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="HUD"))
	TSubclassOf<UUserWidget> BulletWidget;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Animations"))
	UAnimMontage* DrawAnimation1P;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Animations"))
	UAnimMontage* HolsterAnimation1P;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Animations"))
	UAnimMontage* FireAnimation1P;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Animations"))
	UAnimMontage* ReloadAnimation1P;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Animations"))
	UAnimMontage* MeleeAnimation1P;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Sound Effects"))
	USoundBase* DrawSFX;

	UPROPERTY()
	FTimerHandle ReloadTimer;

	UPROPERTY(EditDefaultsOnly)
	UForceFeedbackEffect* FireFeedback;
private:
	UPROPERTY()
	ACharacterBase* CharacterOwner;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ImpactDecal;
};
