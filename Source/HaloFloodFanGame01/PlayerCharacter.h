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
	
public:	
	UFUNCTION()
	virtual void ThrowEquippedGrenade_Implementation() override;
	
	virtual void EquipWeapon(AGunBase* Gun) override;

	virtual void HolsterWeapon(AGunBase* Gun) override;

	virtual void SwitchWeapon() override;

	virtual void Server_SwitchWeapon_Implementation() override;

	virtual void Multi_SwitchWeapon_Implementation() override;
protected:
	// void OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	// End of APawn interface
};

