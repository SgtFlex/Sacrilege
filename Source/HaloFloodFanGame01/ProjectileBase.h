// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "PhysicsInterface.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class ADecalActor;
class UNiagaraSystem;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AProjectileBase : public AActor, public IPhysicsInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	/** Sphere collision component */
public:
	AProjectileBase();

	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(BlueprintNativeEvent)
	void OnProjectileOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void ProjectileStopped(const FVector& StopLocation, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) __override;
	
	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Force;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* IdleSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ExpireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* HitPFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* ImpactDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (DeprecatedProperty))
	TSubclassOf<ADecalActor> ImpactDecalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* IdleSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* TestIdleSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> IgnoreActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer GameplayTags;

private:
	UPROPERTY()
	FTimerHandle DespawnTimerHandle;

};

