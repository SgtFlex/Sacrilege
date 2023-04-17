// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class ABaseGrenade;
class ADecalActor;
class AGunBase;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickupWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropWeapon);

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
	
	virtual float TakePointDamage(float Damage, FVector Force, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void HealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "") override;
	
	UFUNCTION(BlueprintCallable)
	virtual UHealthComponent* GetHealthComponent() override;

	UFUNCTION(BlueprintNativeEvent)
	void Melee();

	UFUNCTION(BlueprintNativeEvent)
	void EquipGrenadeType(TSubclassOf<ABaseGrenade> Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ThrowEquippedGrenade();

	UFUNCTION(BlueprintCallable)
	void UseEquipment();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Pull();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Release();
	
	UFUNCTION(BlueprintCallable)
	virtual void ReloadInput();

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

public:
	UPROPERTY(BlueprintAssignable)
	FOnPickupWeapon OnPickupWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnDropWeapon OnDropWeapon;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere,Category="Stats", meta=(AllowPrivateAccess=true))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, Category="Loadout")
	TSubclassOf<AGunBase> SpawnWeapon;

	UPROPERTY(BlueprintReadOnly)
	class AGunBase* EquippedWep;

	UPROPERTY(BlueprintReadOnly)
	AGunBase* HolsteredWeapon;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* BloodPFX;

	UPROPERTY(EditAnywhere)
	UMaterial* BloodDecalMaterial;

	UPROPERTY(EditAnywhere, Category="Loadout")
	TSubclassOf<ABaseGrenade> EquippedGrenadeClass;

	UPROPERTY(EditAnywhere)
	UAnimMontage* FiringAnim;

	UPROPERTY(EditAnywhere)
	UAnimMontage* HurtAnim;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DeathAnim;

protected:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditAnywhere)
	float MeleeDamage = 30;
	
	UPROPERTY(EditAnywhere)
	float MeleeForce = 100000;
};
