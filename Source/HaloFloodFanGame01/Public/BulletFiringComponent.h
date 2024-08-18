// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "BulletFiringComponent.generated.h"


class UNiagaraSystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HALOFLOODFANGAME01_API UBulletFiringComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBulletFiringComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
    virtual AProjectileBase* FireProjectile(TSubclassOf<AProjectileBase> ProjectileClass, FVector Direction, AActor* Owner, AController* Instigator);

	UFUNCTION(NetMulticast, Unreliable)
	void PlayFX(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	virtual void FireBullet(FHitResult& HitResult, FVector EndLocation);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* FiringVFX;
};
