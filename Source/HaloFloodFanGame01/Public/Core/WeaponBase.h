// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ADecalActor;
class ACharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponPickedUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponDropped);

UENUM(BlueprintType)
enum EHoldType {
	Pistol,
	Rifle,
	Launcher,
	Underhand,
	Sword,
	Hammer,
	Custom
};

UCLASS(Abstract)
class HALOFLOODFANGAME01_API AWeaponBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	
	virtual void Pickup(ACharacterBase* Char);

	virtual void Equip();

	virtual void Drop();

	virtual void Holster();

	UFUNCTION(BlueprintCallable)
	void GetAim(FVector& AimLocation, FVector& AimDirection);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PrimaryFire_Start();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PrimaryFire_End();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SecondaryFire_Start();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SecondaryFire_End();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TertiaryFire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void WeaponMelee();

	UFUNCTION(Reliable, Server)
	void ServerWeaponMelee();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastWeaponMelee();

	UFUNCTION(BlueprintCallable)
	void GetMeleeHit(FHitResult& MeleeHit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DoMeleeHit(const FHitResult MeleeHit);

	UFUNCTION(Server, Reliable)
	void ServerMeleeHit(const FHitResult MeleeHit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDoMeleeHit(const FHitResult MeleeHit); 
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ScopeIn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ScopeOut();

	virtual void OnInteract_Implementation(ACharacterBase* Character) override;
	
	virtual void GetInteractInfo_Implementation(FText& ActionText, FText& ObjectText, UTexture2D*& Icon, ACharacterBase* InteractingCharacter) override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Reload();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadWrite)
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(BlueprintReadOnly)
	APawn* OwningPawn;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ACharacterBase* CharacterOwner;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> ActorsToIgnore;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | General"))
	float DrawSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | General"))
	float HolsterSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Art | HUD"))
	UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Art | HUD"))
	UTexture2D* WeaponIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = "Unnamed Weapon";

	UPROPERTY()
	FText InteractText = FText::FromString("Pickup");

	UPROPERTY()
	UTexture2D* InteractIcon = WeaponIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Art | Animations"))
	UAnimMontage* DrawAnimation1P;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Art | Animations"))
	UAnimMontage* HolsterAnimation1P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Art | Animations"))
	UAnimMontage* MeleeAnimation1P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Art | Animations"))
	USoundBase* MeleeMissSound;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* DrawSFX;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(Category="Art | General"))
	TEnumAsByte<EHoldType> HoldType = EHoldType::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeDamage = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeForce = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeDamageRange = 300;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeLungeRange = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeDelay = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes | Melee"))
	float MeleeCooldownRate = 1;

	UPROPERTY(EditDefaultsOnly, meta=(Category="Art | Effects"))
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<ADecalActor>> MeleeImpactFX;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Art | Effects"))
	TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> MeleeImpactSFX;

	UPROPERTY(BlueprintAssignable)
	FOnWeaponDropped OnWeaponDropped;

	UPROPERTY(BlueprintAssignable)
	FOnWeaponPickedUp OnWeaponPickedUp;	

	UPROPERTY(BlueprintReadOnly)
	bool ScopeActive = false;

protected:
	
	UPROPERTY(EditAnywhere, meta = (Category="Art | General"))
	TSubclassOf<class UUserWidget> ScopeWidget;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* ScopeInSFX;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Art | Effects"))
	USoundBase* ScopeOutSFX;

	//Affects how far we zoom in
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category="Attributes | Gun"))
	float ScopeFOV = 0;
	
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeCooldownHandle1;

private:
	
	UPROPERTY()
	UUserWidget* ScopeOverlay;
	
	UPROPERTY()
	FTimerHandle DrawHandle;

	UPROPERTY()
	FTimerHandle HolsterHandle;

	UPROPERTY()
	FTimerHandle MeleeHitDelayHandle;

	
};