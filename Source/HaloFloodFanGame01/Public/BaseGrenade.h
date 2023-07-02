// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "PickupInterface.h"
#include "GameFramework/Actor.h"
#include "BaseGrenade.generated.h"

class UPickupComponent;
class UNiagaraSystem;
UCLASS()
class HALOFLOODFANGAME01_API ABaseGrenade : public AActor, public IPickupInterface, public IDamageableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseGrenade();

	UPROPERTY(EditAnywhere)
	float FuseTime = 2;

	UPROPERTY(EditAnywhere)
	bool bArmed = false;

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


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void Explode();

	void SetArmed(bool NewArmed = false);

	void Arm(float ArmTime = 4);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Pickup(AHaloFloodFanGame01Character* Character) override;

	// virtual float TakePointDamage(FPointDamageEvent const& PointDamageEvent, FVector Force, AController*, AActor* DamageCauser) override;

	//virtual float TakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual float TakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
