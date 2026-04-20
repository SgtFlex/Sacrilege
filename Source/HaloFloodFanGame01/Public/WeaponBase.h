// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ADecalActor;
class ACharacterBase;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

	UFUNCTION(BlueprintCallable)
	void GetMeleeHit(FHitResult& MeleeHit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DoMeleeHit(const FHitResult MeleeHit); 

	virtual void OnInteract_Implementation(ACharacterBase* Character) override;

	// virtual void GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon, ACharacterBase* InteractingCharacter) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Reload();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float DrawSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Attributes"))
	float HolsterSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="HUD"))
	UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="HUD"))
	UTexture2D* WeaponIcon;

	UPROPERTY()
	FText InteractText = FText::FromString("Pickup");

	UPROPERTY()
	UTexture2D* InteractIcon = WeaponIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Animations"))
	UAnimMontage* DrawAnimation1P;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Animations"))
	UAnimMontage* HolsterAnimation1P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Animations"))
	UAnimMontage* MeleeAnimation1P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Category="Animations"))
	USoundBase* MeleeMissSound;
	
	UPROPERTY(EditDefaultsOnly, meta = (Category="Sound Effects"))
	USoundBase* DrawSFX;

	UPROPERTY(BlueprintReadOnly)
	APawn* OwningPawn;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ACharacterBase* CharacterOwner;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EHoldType> HoldType = EHoldType::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeDamage = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeForce = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeDamageRange = 300;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeLungeRange = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeDelay = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category="Melee"))
	float MeleeCooldownRate = 1;

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<ADecalActor>> MeleeImpactFX;

private:

protected:
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle DrawHandle;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle HolsterHandle;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeHitDelayHandle;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeCooldownHandle1;
};
