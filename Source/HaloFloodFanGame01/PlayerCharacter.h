// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "Core/CharacterBase.h"
#include "PlayerCharacter.generated.h"

enum class EHardwareDevicePrimaryType : uint8;
class UInteractableInterface;
class IInteractableInterface;
class USphereComponent;
class UBoxComponent;
class UPlayerHUD;
class AGrenadeBase;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, Interactable);



UCLASS(config=Game)
class APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()	
public:
	APlayerCharacter();
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintGetter)
	void GetPlayerAim(FHitResult& HitResult) const;

	float AimAssist() const;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	UFUNCTION()
	virtual void ThrowEquippedGrenade_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void Interact();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_Interact();
	
	UFUNCTION(BlueprintCallable)
	void SwitchGrenadeType();

	void SwitchGrenadeType(int Index);
	
	virtual void NotifyRestarted() override;
	
	virtual void UnPossessed() override;
	
	virtual void EquipWeapon(AGunBase* Gun) override;

	virtual void HolsterWeapon(AGunBase* Gun) override;

	virtual void SwitchWeapon() override;

	virtual void Server_SwitchWeapon_Implementation() override;

	virtual void Multi_SwitchWeapon_Implementation() override;
	
	virtual void ScopeWeapon();
	
	/** Returns Mesh1P subobject **/
	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetMesh1P() const override { return Mesh1P; }
	
	/** Returns FirstPersonCameraComponent subobject **/
	UFUNCTION(BlueprintCallable)
	virtual UCameraComponent* GetFirstPersonCameraComponent() const override { return FirstPersonCameraComponent; }

	virtual UUserWidget* GetPlayerHUD() const override { return PlayerHUD; }
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(Server, Unreliable)
	void Server_Look(float Pitch);

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_Look(float Pitch);

	UFUNCTION()
	void SetCurrentInteractable();

	UFUNCTION(BlueprintCallable)
	virtual void Melee_Implementation() override;

	UFUNCTION()
	virtual void MeleeDamageCode();
	UFUNCTION()
	void MeleeUpdate(float Alpha);

	void OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
	
public:
	UPROPERTY(BlueprintReadOnly)
	APlayerController* PlayerController;
	
	UPROPERTY()
	FOnInteractableChanged OnInteractableChanged;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY(BlueprintReadWrite)
	class UUserWidget* PlayerHUD;

	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 FragCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 PlasmaCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 SpikeCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 IncenCount = 0;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ThrowGrenadeAnimation1P;

	UPROPERTY()
	TArray<AActor*> InteractableActors;

private:
	UPROPERTY()
	AActor* InteractableActor;
	
	FTimerHandle ShieldDelayTimerHandle;

	FTimeline MeleeTimeline;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* MeleeCurve;

	FHitResult PlayerAim;

	UPROPERTY()
	UInputDeviceSubsystem* InputDeviceSubsystem;
};

