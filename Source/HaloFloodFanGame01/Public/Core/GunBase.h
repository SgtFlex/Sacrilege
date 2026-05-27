// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"
#include "FGunAIBehavior.h"
#include "GunBase.generated.h"

struct FGunAIBehavior;
class UBulletFiringComponent;
class ACharacterBase;
class AProjectileBase;
class UPhysicalMaterial;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAmmoUpdated);

UCLASS(Abstract)
class HALOFLOODFANGAME01_API AGunBase : public AWeaponBase
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	AGunBase();

public:	
	virtual void SecondaryFire_Start_Implementation() override;

	virtual void PrimaryFire_Start_Implementation() override;

	virtual void PrimaryFire_End_Implementation() override;

	virtual void Reload_Implementation() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	bool CanFire();

	// UFUNCTION(BlueprintCallable)
	// void GetAim(FVector& AimLocation, FVector& AimDirection);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SpawnBullet();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* SpawnProjectile(TSubclassOf<AActor> ProjToSpawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* SpawnProjectileInDirection(TSubclassOf<AActor> ProjToSpawn, FVector Direction);

	UFUNCTION(NetMulticast, Unreliable)
	void PlayFireFX();

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnTrailFX(FHitResult Hit);
	
	UFUNCTION(BlueprintImplementableEvent)
	void K2_SpawnTrailFX(FHitResult Hit);

	UFUNCTION()
	void UpdateMagazineElements();
	

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnFire OnFire;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnReload OnReload;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnAmmoUpdated OnAmmoUpdated;	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Attributes | Gun")
	UBulletFiringComponent* BulletFiringComponent;
	
	// //Damage of the hitscan applied to the hit actor
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass", DeprecatedProperty))
	// float Damage = 15;

	// //Force of the hitscan applied to the hit component if it simulates physics
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass", DeprecatedProperty))
	// float Force = 1000;

	//Fire rate of the gun in bullets per minute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"))
	float FireRate = 500;

	//Amount of hitscan/projectiles that are fired simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"))
	int32 MultiShot = 1;

	//Controls how many bullets are fired when the trigger is pulled. 0 means full auto, 1 means semi auto, 2+ is burst fire of whatever amount is provided
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	int32 BurstAmount = 0; 

	//The minimum delay between bursts if BurstAmount >= 2
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	float BurstRetriggerDelay = 0.15f;

	//Amount of seconds required to reload the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"))
	float ReloadSpeed = 3;
	
	// //Range of the hitscan trace
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass", DeprecatedProperty))
	// float Range = 5000;
	//
	// UPROPERTY(EditAnywhere, meta = (Category="Attributes|Hitscan", EditCondition="!ProjectileClass", DeprecatedProperty))
	// UCurveFloat* FalloffCurve;

	//The maximum bullets in a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"))
	int32 MaxMagazine = 32;

	//The amount of bullets the weapon spawns with in a magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"), ReplicatedUsing=UpdateMagazineElements)
	int32 CurMagazine = MaxMagazine;

	//The maximum amount of bullets the weapon can have in reserve.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"))
	int32 MaxReserve = 160;

	//The amount of bullets this weapon spawns with in reserve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Gun"), ReplicatedUsing=UpdateMagazineElements)
	int32 CurReserve = MaxReserve;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | General"))
	TSubclassOf<UUserWidget> BulletWidget;

protected:
	
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	FGunAIBehavior GunAIBehavior;
	
	//Actor that will be spawned from the barrel of the gun. Otherwise uses hitscan properties below
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	TSubclassOf<AActor> ProjectileClass;
	
	//How far left/right the gun jumps when firing a bullet
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	float HorizontalRecoil = 1;

	//How far up/down the gun jumps when firing a bullet
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	float VerticalRecoil = 1;

	//The degrees of horizontal spread of projectiles/hitscan. 90 means anywhere from the left to the front to the right.
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	float HorizontalSpread = 0;

	//The degrees of vertical spread of projectiles/hitscan. 90 means anywhere from the down to the front to the up.
	UPROPERTY(EditAnywhere, meta = (Category="Attributes | Gun"))
	float VerticalSpread = 0;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* FiringSound;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	class UNiagaraSystem* MuzzlePFX;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	class UNiagaraSystem* TrailPFX;
	
		

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Animations"))
	UAnimMontage* FireAnimation1P;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Animations"))
	UAnimMontage* ReloadAnimation1P;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	UForceFeedbackEffect* FireFeedback;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	TSubclassOf<UCameraShakeBase> FiringCameraShake;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<AActor>> ImpactFXMap;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	TSubclassOf<AActor> ImpactDecal;	
	
private:
	
	int32 BulletsFired;
	
	UPROPERTY()
	FTimerHandle FireHandle;

	UPROPERTY()
	FTimerHandle BurstRetriggerHandle;

	bool bReloading = false;

	UPROPERTY(Replicated)
	bool bFiring = false;
	
	UPROPERTY()
	FTimerHandle ReloadTimer;
};
