// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/PointLight.h"
#include "BulletFiringComponent.generated.h"


class APointLight;
class AProjectileBase;
class UBullet;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBulletFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBulletHit, FHitResult, HitResult);


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )



class HALOFLOODFANGAME01_API UBulletFiringComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBulletFiringComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	

	UFUNCTION(NetMulticast, Unreliable)
	void PlayFX(FVector Location, FRotator Rotation);

	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IgnoreActors"))
	virtual AProjectileBase* FireProjectile(TSubclassOf<AProjectileBase> ProjectileClass, FVector Direction, AActor* Owner, AController* Instigator, const TArray<
	                                        AActor*>& IgnoreActors);

	UFUNCTION(BlueprintCallable)
	virtual void FireBullet(FHitResult& HitResult, FVector AimLocation, FVector Direction,  TArray<AActor*> ActorsToIgnore, AActor* DamageCauser, AController* EventInstigator);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* FiringVFX;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DeprecatedProperty))
	// float HitScanRange = 5000;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DeprecatedProperty))
	// float HitScanDamage = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DeprecatedProperty))
	UCurveFloat* HitScanFalloffCurve;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DeprecatedProperty))
	// float HitScanForce = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APointLight> PointLightClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalSpread = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalSpread = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBullet> BulletInfo;

	UPROPERTY(BlueprintAssignable)
	FOnBulletFired OnBulletFired;

	UPROPERTY(BlueprintAssignable)
	FOnBulletHit OnBulletHit;
};
