// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "HaloFloodFanGame01Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AHaloFloodFanGame01Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PrimaryAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SecondaryAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;
	
public:
	AHaloFloodFanGame01Character();

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

	void PrimaryInput();

	void SecondaryInput();

	void ReloadInput();

	void ThrowGrenade();

	void UseEquipment();

	virtual void Death();

	virtual void StartShieldRegen();

	virtual void RegenShield();

	virtual void BreakShield();

	void SwitchWeapon();

	void Interact();
private:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Attack();

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	UPROPERTY(EditAnywhere, Category="Collision")
	TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_Pawn;

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY(EditAnywhere)
		TSubclassOf<class UHaloHUDWidget> PlayerHUDClass;

	UPROPERTY()
		class UHaloHUDWidget* PlayerHUD;
	

	UPROPERTY(EditAnywhere)
	AGunBase* EquippedWep;

	UPROPERTY(EditAnywhere)
	AGunBase* HolsteredWeapon;

	TArray<int> Guns;

	float Health = 100;
	float MaxHealth = 100;
	float HealthArmor = 0;
	float MaxHealthArmor = 100;

	float Shields = 100;
	float MaxShields = 100;
	float ShieldRegenDelay = 3;
	float ShieldRegenRatePerSecond = 30;
	float ShieldRegenTickRate = 0.01;
	float ShieldArmor = 0;
	float MaxShieldArmor = 100;

	int32 FragCount = 0;
	int32 PlasmaCount = 0;
	int32 SpikeCount = 0;
	int32 IncenCount = 0;
	
	bool CanShieldsRecharge = false;

private:
	FTimerHandle ShieldDelayTimerHandle;
};

