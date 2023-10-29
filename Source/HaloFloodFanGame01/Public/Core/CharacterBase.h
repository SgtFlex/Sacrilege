// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

class ASmartObject;
class AGrenadeBase;
class ADecalActor;
class AGunBase;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickupWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropWeapon);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKilled, AController*, Instigator, AActor*, Causer);
DECLARE_DELEGATE_OneParam(FTest, ACharacterBase*);


USTRUCT()
struct FGrenadeStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGrenadeBase> GrenadeClass;
	
	UPROPERTY(EditAnywhere)
	int GrenadeAmount;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeInvetoryUpdated, TArray<FGrenadeStruct>, GrenadeInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeTypeSwitched, TSubclassOf<AGrenadeBase>, GrenadeClass);

UCLASS()
class HALOFLOODFANGAME01_API ACharacterBase : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
	float CustomOnTakeAnyDamage(float DamageAmount, FVector Force, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintNativeEvent)
	float CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );
	
	virtual UHealthComponent* GetHealthComponent() override;

	UFUNCTION(BlueprintNativeEvent)
	void Melee();

	UFUNCTION(BlueprintNativeEvent)
	void EquipGrenadeType(TSubclassOf<AGrenadeBase> Grenade);

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

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	virtual void Server_PickupWeapon(AGunBase* Gun);
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    virtual void Multi_PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

	UFUNCTION()
	void RagdollSettled();

	void Stun();

	void Unstun();

public:
	UPROPERTY(BlueprintAssignable)
	FOnPickupWeapon OnPickupWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnDropWeapon OnDropWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeInvetoryUpdated OnGrenadeInvetoryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeTypeSwitched OnGrenadeTypeSwitched;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Stats", meta=(AllowPrivateAccess=true))
	UHealthComponent* HealthComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Loadout", meta=(AllowPrivateAccess=true))
	TSubclassOf<AGunBase> SpawnWeaponClass;

	
	UPROPERTY(EditAnywhere, Category="Loadout")
	TSubclassOf<class AGunBase> HolsteredWeaponClass;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* HolsteredWeapon;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* BloodPFX;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodDecalMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodSplatterMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* DeathSound;
	//
	// UPROPERTY(EditAnywhere, Category="Loadout")
	// TSubclassOf<ABaseGrenade> EquippedGrenadeClass;

	UPROPERTY(EditAnywhere)
	TArray<FGrenadeStruct> GrenadeInventory;

	UPROPERTY(EditAnywhere)
	UAnimMontage* FiringAnim;

	UPROPERTY(EditAnywhere)
	UAnimMontage* HurtAnim;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DeathAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObject;

	UPROPERTY(VisibleAnywhere)
	FOnKilled OnKilled;

	UPROPERTY()
	FTimerHandle StunTimer;

	UPROPERTY()
	float StunAmount = 100;

	//static FTest TestDelegate;

	int CurGrenadeTypeI = 0;

protected:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditAnywhere)
	float MeleeDamage = 30;
	
	UPROPERTY(EditAnywhere)
	float MeleeForce = 100000;

	FTimerHandle RagdollTimer;
};
