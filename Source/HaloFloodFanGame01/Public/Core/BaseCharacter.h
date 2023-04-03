// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Particles/ParticleSystem.h"
#include "BaseCharacter.generated.h"

class AGunBase;
class UHealthComponent;
UCLASS()
class HALOFLOODFANGAME01_API ABaseCharacter : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual float TakePointDamage(float Damage, FVector Force, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual float TakeRadialDamage(float Damage, FVector Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	// virtual void TakeDamage(float DamageAmount) override;
	
	virtual void HealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "") override;

	UFUNCTION(BlueprintNativeEvent)
	void Melee();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Pull();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Release();

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere,Category="Stats", meta=(AllowPrivateAccess=true))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AGunBase> SpawnWeapon;
	
	AGunBase* EquippedWep;
	AGunBase* HolsteredWeapon;

private:
	

protected:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditAnywhere)
	float MeleeDamage = 30;
	UPROPERTY(EditAnywhere)
	float MeleeForce = 100000;
};
