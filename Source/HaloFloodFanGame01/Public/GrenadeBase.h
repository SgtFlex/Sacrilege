// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "PickupInterface.h"
#include "GameFramework/Actor.h"
#include "GrenadeBase.generated.h"

class UImage;
class UProjectileMovementComponent;
class UPickupComponent;
class UNiagaraSystem;
UCLASS()
class HALOFLOODFANGAME01_API AGrenadeBase : public AActor, public IPickupInterface, public IDamageableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeBase();

	UPROPERTY(EditAnywhere)
	float FuseTime = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn))
	bool bArmed = false;

	bool FuseStarted = false;

	UPROPERTY(EditAnywhere)
	float InnerExplosionRadius = 300;
	
	UPROPERTY(EditAnywhere)
	float OuterExplosionRadius = 1000;

	UPROPERTY(EditAnywhere)
	float ExplosionDamageFalloff = 5;

	UPROPERTY(EditAnywhere)
	float MinExplosionDamage = 25;
	
	UPROPERTY(EditAnywhere)
	float MaxExplosionDamage = 150;

	UPROPERTY(EditAnywhere)
	float ExplosionForce = 50000;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* ExplosionPFX;

	UPROPERTY(EditAnywhere)
	USoundBase* ExplosionSFX;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	FTimerHandle FuseTimer;

	UPROPERTY(EditAnywhere)
	UPickupComponent* PickupComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	UTexture2D* GrenadeIcon;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void Explode();

	void SetArmed(bool NewArmed = false);

	void StartFuse(float NewFuseTime = 4);

	UFUNCTION(BlueprintNativeEvent)
	void OnCollide(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );
	
	UFUNCTION()
	virtual void Pickup(APlayerCharacter* Character) override;

	// virtual float TakePointDamage(FPointDamageEvent const& PointDamageEvent, FVector Force, AController*, AActor* DamageCauser) override;

	//virtual float TakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	virtual float CustomOnTakeAnyDamage(float DamageAmount, FVector Force, AController* EventInstigator, AActor* DamageCauser) override;
};
