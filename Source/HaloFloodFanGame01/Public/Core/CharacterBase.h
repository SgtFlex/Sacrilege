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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponsUpdated, AGunBase*, NewGun, AGunBase*, OldGun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKilled, ACharacterBase*, Character, AController*, Instigator, AActor*, Causer);


USTRUCT(BlueprintType)
struct FGrenadeStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGrenadeBase> GrenadeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	
	UFUNCTION(BlueprintNativeEvent)
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintNativeEvent)
	void DropGrenades();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );

	UFUNCTION(BlueprintCallable)
	void SetSmartObject(ASmartObject* NewSmartObject);
	
	virtual UHealthComponent* GetHealthComponent() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Melee();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanMelee();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EquipGrenadeType(TSubclassOf<AGrenadeBase> Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ThrowEquippedGrenade();

	UFUNCTION(BlueprintCallable)
	virtual void UseEquipment();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Pull();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Release();
	
	UFUNCTION(BlueprintCallable)
	virtual void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeapon();

	UFUNCTION()
	virtual void EquipWeapon(AGunBase* Gun);

	UFUNCTION()
	virtual void HolsterWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AGunBase* Gun);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	virtual void Server_PickupWeapon(AGunBase* Gun);
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    virtual void Multi_PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

	UFUNCTION()
	void RagdollSettled(UPrimitiveComponent* Component, FName Name);

	UFUNCTION(BlueprintCallable)
	virtual void Stun(float StunTime = 1);

	void Unstun();

public:
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FWeaponsUpdated WeaponsUpdated;
	
	UPROPERTY(BlueprintAssignable)
	FOnPickupWeapon OnPickupWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnDropWeapon OnDropWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnKilled OnKilled;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeInvetoryUpdated OnGrenadeInventoryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeTypeSwitched OnGrenadeTypeSwitched;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;

	//Loadout
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0))
	TSubclassOf<AGunBase> EquippedWeaponClass;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0))
	TSubclassOf<AGunBase> HolsteredWeaponClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0))
	TArray<FGrenadeStruct> GrenadeInventory;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* HolsteredWeapon;

	//Blood
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* BloodPFX;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodDecalMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodSplatterMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* DeathSound;

	//Anims
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FiringAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* HurtAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* DeathAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObject;
	
	UPROPERTY()
	FTimerHandle StunTimer;

	UPROPERTY()
	float StunAmount = 100;

	int CurGrenadeTypeI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	uint8 TeamId = 0;
private:
	
protected:
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeDamage = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeForce = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeCooldown = 1;

	FTimerHandle RagdollTimer;
};
